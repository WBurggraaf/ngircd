/*
 * ngircd_daemon.c
 *
 * Daemon bootstrap helpers that must live in ngircd_impl.dll so they can
 * access the NGIRCd_* globals defined in the same DLL.
 * Content moved from archive/upstream-ngircd/src/ngircd/platform/ngircd.c.
 *
 * Uses Conf_* globals directly (they are in the same DLL) rather than going
 * through config_get_api_v1(), which would create a circular DLL dependency
 * (config.dll -> ngircd_impl.dll -> config.dll).
 */

#define NGIRCD_IMPL_BUILD
#include "portab.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>

#ifdef _WIN32
#include "config.h"
#include "win32compat.h"
#endif

#include "conn.h"
#include "conf.h"
#include "log.h"
#include "ngircd.h"


/**
 * Initialize the server start time globals.
 * Called by server_app to avoid cross-DLL data variable access.
 */
NGIRCD_FUNC void
ngircd_set_start_time(void)
{
	NGIRCd_Start = time(NULL);
	(void)strftime(NGIRCd_StartStr, sizeof(NGIRCd_StartStr),
		       "%a %b %d %Y at %H:%M:%S (%Z)",
		       localtime(&NGIRCd_Start));
}


#if !defined(SINGLE_USER_OS)

static bool
NGIRCd_getNobodyID(uid_t *uid, gid_t *gid)
{
	struct passwd *pwd;

#ifdef __CYGWIN__
	if (geteuid() && getuid()) {
		*uid = getuid();
		*gid = getgid();
		return true;
	}
#endif

	pwd = getpwnam("nobody");
	if (!pwd)
		return false;

	if (!pwd->pw_uid || !pwd->pw_gid)
		return false;

	*uid = pwd->pw_uid;
	*gid = pwd->pw_gid;
	endpwent();

	return true;
}

#endif


static void
Setup_FDStreams(int fd)
{
	if (fd < 0)
		return;

	fflush(stdout);
	fflush(stderr);

	dup2(fd, 0); dup2(fd, 1); dup2(fd, 2);
}


NGIRCD_FUNC bool
NGIRCd_Init(bool NGIRCd_NoDaemon)
{
	static bool initialized;
	bool chrooted = false;
	struct passwd *pwd;
	struct group *grp;
	int real_errno, fd = -1;
	pid_t pid;
	uid_t effective_uid;
	gid_t effective_gid;

	LogDebug("NGIRCd_Init(%d): starting.", NGIRCd_NoDaemon);

	if (initialized)
		return true;

	if (!NGIRCd_NoDaemon) {
		fd = open("/dev/null", O_RDWR);
		if (fd < 0)
			Log(LOG_WARNING, "Could not open /dev/null: %s",
			    strerror(errno));
	}

#ifdef SSL_SUPPORT
	if (!ConnSSL_InitLibrary()) {
		Log(LOG_ERR, "Error during SSL initialization!");
		goto out;
	}
#endif

	if (Conf_Chroot[0]) {
		if (chdir(Conf_Chroot) != 0) {
			Log(LOG_ERR, "Can't chdir() in ChrootDir (%s): %s!",
			    Conf_Chroot, strerror(errno));
			goto out;
		}

		if (chroot(Conf_Chroot) != 0) {
			Log(LOG_ERR,
			    "Can't change root directory to \"%s\": %s!",
			    Conf_Chroot, strerror(errno));
			goto out;
		} else {
			chrooted = true;
			Log(LOG_INFO,
			    "Changed root and working directory to \"%s\".",
			    Conf_Chroot);
		}
	}

#if !defined(SINGLE_USER_OS)
	if (Conf_UID == 0) {
		uid_t nobody_uid = 0;
		gid_t nobody_gid = 0;

		pwd = getpwuid(0);
		Log(LOG_INFO,
		    "ServerUID must not be %s(0), using \"nobody\" instead.",
		    pwd ? pwd->pw_name : "?");
		if (!NGIRCd_getNobodyID(&nobody_uid, &nobody_gid)) {
			Log(LOG_WARNING,
			    "Could not get user/group ID of user \"nobody\": %s",
			    errno ? strerror(errno) : "not found");
			goto out;
		}
		Conf_UID = nobody_uid;
		Conf_GID = nobody_gid;
	}

	effective_uid = Conf_UID;
	effective_gid = Conf_GID;

	if (getgid() != Conf_GID) {
		const gid_t gid = Conf_GID;

		if (setgid(gid) != 0) {
			real_errno = errno;
			grp = getgrgid(gid);
			Log(LOG_ERR, "Can't change group ID to %s(%u): %s!",
			    grp ? grp->gr_name : "?", gid,
			    strerror(real_errno));
			if (real_errno != EPERM && real_errno != EINVAL)
				goto out;
		}
#ifdef HAVE_SETGROUPS
		if (setgroups(0, NULL) != 0) {
			real_errno = errno;
			Log(LOG_ERR, "Can't drop supplementary group IDs: %s!",
			    strerror(errno));
			if (real_errno != EPERM)
				goto out;
		}
#else
		Log(LOG_WARNING,
		    "Can't drop supplementary group IDs: setgroups(3) missing!");
#endif
	}

	if (getuid() != Conf_UID) {
		const uid_t uid = Conf_UID;

		if (setuid(uid) != 0) {
			real_errno = errno;
			pwd = getpwuid(uid);
			Log(LOG_ERR, "Can't change user ID to %s(%u): %s!",
			    pwd ? pwd->pw_name : "?", uid,
			    strerror(real_errno));
			if (real_errno != EPERM && real_errno != EINVAL)
				goto out;
		}
	}
#else
	effective_uid = getuid();
	effective_gid = getgid();
#endif /* !SINGLE_USER_OS */

	initialized = true;

	if (!NGIRCd_NoDaemon) {
		pid = fork();
		if (pid > 0) {
			exit(0);
		}
		if (pid < 0) {
			fprintf(stderr,
				"%s: Can't fork: %s!\nFatal error, exiting now ...\n",
				PACKAGE_NAME, strerror(errno));
			exit(1);
		}

#ifdef HAVE_SETSID
		(void)setsid();
#else
		setpgrp(0, getpid());
#endif
		if (chdir("/") != 0)
			Log(LOG_ERR, "Can't change directory to '/': %s!",
			    strerror(errno));

		Setup_FDStreams(fd);
		if (fd > 2)
			close(fd);
	}
	pid = getpid();

	effective_uid = getuid();
	effective_gid = getgid();

	pwd = getpwuid(effective_uid);
	grp = getgrgid(effective_gid);

	Log(LOG_INFO, "Running as user %s(%ld), group %s(%ld), with PID %ld.",
	    pwd ? pwd->pw_name : "unknown", (long)effective_uid,
	    grp ? grp->gr_name : "unknown", (long)effective_gid, (long)pid);

	if (chrooted) {
		Log(LOG_INFO, "Running with root directory \"%s\".",
		    Conf_Chroot);
		return true;
	} else
		Log(LOG_INFO, "Not running with changed root directory.");

	if (NGIRCd_NoDaemon)
		return true;

	if (pwd) {
		if (chdir(pwd->pw_dir) == 0)
			LogDebug("Changed working directory to \"%s\" ...",
				 pwd->pw_dir);
		else
			Log(LOG_ERR,
			    "Can't change working directory to \"%s\": %s!",
			    pwd->pw_dir, strerror(errno));
	} else
		Log(LOG_ERR, "Can't get user information for UID %d!?",
		    (int)effective_uid);

	return true;
 out:
	if (fd > 2)
		close(fd);
	return false;
}
