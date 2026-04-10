/*
 * ngIRCd -- The Next Generation IRC Daemon
 * Copyright (c)2001-2024 Alexander Barton (alex@barton.de) and Contributors.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * Please read the file COPYING, README and AUTHORS for more information.
 */

/*
 * This file is the thin EXE entry point.  It does NOT define NGIRCd_* globals
 * (those live in ngircd_impl.dll via ngircd_runtime_globals.c) and does NOT
 * contain NGIRCd_Init (that lives in ngircd_daemon.c, also in ngircd_impl.dll).
 * All NGIRCd_* symbols are imported from ngircd_impl.dll at runtime.
 */
#include "portab.h"

/**
 * @file
 * The main program, including the C function main() which is called
 * by the loader of the operating system.
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>

#if defined(DEBUG) && defined(HAVE_MTRACE)
#include <mcheck.h>
#endif

#include "conn.h"
#include "class.h"
#include "channel.h"
#include "log.h"
#include "sighandlers.h"

#include "host/host.h"
#include "config/config.h"
#include "server_app/server_app.h"

#include "ngircd.h"

static void Show_Version PARAMS(( void ));
static void Show_Help PARAMS(( void ));

static void Fill_Version PARAMS(( void ));

/**
 * The main() function of ngIRCd.
 *
 * Here all starts: this function is called by the operating system loader,
 * it is the first portion of code executed of ngIRCd.
 *
 * @param argc The number of arguments passed to ngIRCd on the command line.
 * @param argv An array containing all the arguments passed to ngIRCd.
 * @return Global exit code of ngIRCd, zero on success.
 */
GLOBAL int
main(int argc, const char *argv[])
{
	bool ok, configtest = false;
	bool NGIRCd_NoDaemon = false, NGIRCd_NoSyslog = false;
	int i;
	size_t n;
	host_module_set_t module_set;
	const host_wiring_t *wiring;
	const config_api_t *config_api;
	const server_app_api_t *server_app_api;

#if defined(DEBUG) && defined(HAVE_MTRACE)
	/* enable GNU libc memory tracing when running in debug mode
	 * and functionality available */
	mtrace();
#endif

	umask(0077);

	NGIRCd_SignalQuit = NGIRCd_SignalRestart = false;
	NGIRCd_Passive = false;
	NGIRCd_Debug = false;
#ifdef SNIFFER
	NGIRCd_Sniffer = false;
#endif

	Fill_Version();

	module_set.core_runtime = NULL;
	module_set.logging = logging_get_api_v1();
	module_set.config = config_get_api_v1();
	module_set.platform = platform_get_api_v1();
	module_set.net_transport = net_transport_get_api_v1();
	module_set.resolver = resolver_get_api_v1();
	wiring = host_get_wiring_v1();
	if (!wiring || wiring->wire(&module_set) != CORE_STATUS_OK) {
		fprintf(stderr, "%s: host wiring validation failed!\n", PACKAGE_NAME);
		exit(1);
	}
	server_app_api = server_app_get_api_v1();
	if (!server_app_api || server_app_api->api_major != 1u) {
		fprintf(stderr, "%s: server_app API mismatch!\n", PACKAGE_NAME);
		exit(1);
	}
	config_api = config_get_api_v1();

	/* parse conmmand line */
	for (i = 1; i < argc; i++) {
		ok = false;
		if (argv[i][0] == '-' && argv[i][1] == '-') {
			/* long option */
			if (strcmp(argv[i], "--config") == 0) {
				if (i + 1 < argc) {
					/* Ok, there's an parameter left */
					strlcpy(NGIRCd_ConfFile, argv[i+1],
						sizeof(NGIRCd_ConfFile));
					/* next parameter */
					i++; ok = true;
				}
			}
			if (strcmp(argv[i], "--configtest") == 0) {
				configtest = true;
				ok = true;
			}
			if (strcmp(argv[i], "--debug") == 0) {
				NGIRCd_Debug = true;
				ok = true;
			}
			if (strcmp(argv[i], "--help") == 0) {
				Show_Version();
				puts(""); Show_Help( ); puts( "" );
				exit(0);
			}
			if (strcmp(argv[i], "--nodaemon") == 0) {
				NGIRCd_NoDaemon = true;
				NGIRCd_NoSyslog = true;
				ok = true;
			}
			if (strcmp(argv[i], "--passive") == 0) {
				NGIRCd_Passive = true;
				ok = true;
			}
#ifdef SNIFFER
			if (strcmp(argv[i], "--sniffer") == 0) {
				NGIRCd_Sniffer = true;
				ok = true;
			}
#endif
#ifdef SYSLOG
			if (strcmp(argv[i], "--syslog") == 0) {
				NGIRCd_NoSyslog = false;
				ok = true;
			}
#endif
			if (strcmp(argv[i], "--version") == 0) {
				Show_Version();
				exit(0);
			}
		}
		else if(argv[i][0] == '-' && argv[i][1] != '-') {
			/* short option */
			for (n = 1; n < strlen(argv[i]); n++) {
				ok = false;
				if (argv[i][n] == 'd') {
					NGIRCd_Debug = true;
					ok = true;
				}
				if (argv[i][n] == 'f') {
					if (!argv[i][n+1] && i+1 < argc) {
						/* Ok, next character is a blank */
						strlcpy(NGIRCd_ConfFile, argv[i+1],
							sizeof(NGIRCd_ConfFile));

						/* go to the following parameter */
						i++;
						n = strlen(argv[i]);
						ok = true;
					}
				}

				if (argv[i][n] == 'h') {
					Show_Version();
					puts(""); Show_Help(); puts("");
					exit(1);
				}

				if (argv[i][n] == 'n') {
					NGIRCd_NoDaemon = true;
					NGIRCd_NoSyslog = true;
					ok = true;
				}
				if (argv[i][n] == 'p') {
					NGIRCd_Passive = true;
					ok = true;
				}
#ifdef SNIFFER
				if (argv[i][n] == 's') {
					NGIRCd_Sniffer = true;
					ok = true;
				}
#endif
				if (argv[i][n] == 't') {
					configtest = true;
					ok = true;
				}

				if (argv[i][n] == 'V') {
					Show_Version();
					exit(1);
				}
#ifdef SYSLOG
				if (argv[i][n] == 'y') {
					NGIRCd_NoSyslog = false;
					ok = true;
				}
#endif

				if (!ok) {
					fprintf(stderr,
						"%s: invalid option \"-%c\"!\n",
						PACKAGE_NAME, argv[i][n]);
					fprintf(stderr,
						"Try \"%s --help\" for more information.\n",
						PACKAGE_NAME);
					exit(2);
				}
			}

		}
		if (!ok) {
			fprintf(stderr, "%s: invalid option \"%s\"!\n",
				PACKAGE_NAME, argv[i]);
			fprintf(stderr, "Try \"%s --help\" for more information.\n",
				PACKAGE_NAME);
			exit(2);
		}
	}

	/* Debug level for "VERSION" command */
	NGIRCd_DebugLevel[0] = '\0';
	if (NGIRCd_Debug)
		strcpy(NGIRCd_DebugLevel, "1");
#ifdef SNIFFER
	if (NGIRCd_Sniffer) {
		NGIRCd_Debug = true;
		strcpy(NGIRCd_DebugLevel, "2");
	}
#endif

	if (configtest) {
		Show_Version(); puts("");
		if (!config_api || !config_api->test)
			exit(1);
		exit(config_api->test());
	}

	if (server_app_api->create() != CORE_STATUS_OK
	    || server_app_api->start() != CORE_STATUS_OK) {
		fprintf(stderr, "%s: server_app startup failed!\n", PACKAGE_NAME);
		exit(1);
	}

	while (!NGIRCd_SignalQuit) {
		NGIRCd_SignalRestart = false;
		NGIRCd_SignalQuit = false;

		if (server_app_api->init_runtime() != CORE_STATUS_OK) {
			Log(LOG_ALERT, "Fatal: Could not initialize server runtime!");
			exit(1);
		}

		if (server_app_api->init_random() != CORE_STATUS_OK) {
			Log(LOG_ALERT, "Fatal: Could not initialize random state!");
			exit(1);
		}
		if (server_app_api->init_preloop(!NGIRCd_NoSyslog) != CORE_STATUS_OK) {
			Log(LOG_ALERT, "Fatal: Could not initialize server pre-loop state!");
			exit(1);
		}

		/* Initialize the "main program":
		 * chroot environment, user and group ID, ... */
		if (server_app_api->init_daemon((int)NGIRCd_NoDaemon) != CORE_STATUS_OK) {
			Log(LOG_ALERT, "Fatal: Initialization failed, exiting!");
			exit(1);
		}

		if (server_app_api->init_runtime_loop() != CORE_STATUS_OK) {
			Log(LOG_ALERT,
			    "Fatal: Could not initialize IO and runtime loop!");
			exit(1);
		}

		/* Create protocol and server identification. The syntax
		 * used by ngIRCd in PASS commands and the known "extended
		 * flags" are described in doc/Protocol.txt. */
		if (server_app_api->build_proto_id(
			    NGIRCd_ProtoID, sizeof NGIRCd_ProtoID) != CORE_STATUS_OK) {
			Log(LOG_ALERT, "Fatal: Could not build protocol ID!");
			exit(1);
		}
		LogDebug("Protocol and server ID is \"%s\".", NGIRCd_ProtoID);

		if (server_app_api->init_predefined_channels() != CORE_STATUS_OK) {
			Log(LOG_ALERT, "Fatal: Could not initialize predefined channels!");
			exit(1);
		}

		if (server_app_api->init_listeners() != CORE_STATUS_OK) {
			Log(LOG_ALERT,
			    "Server isn't listening on a single port!" );
			Log(LOG_ALERT,
			    "%s exiting due to fatal errors!", PACKAGE_NAME);
			(void)server_app_api->delete_pidfile();
			exit(1);
		}

		if (server_app_api->create_pidfile() != CORE_STATUS_OK) {
			Log(LOG_ALERT, "Fatal: Could not create PID file!");
			exit(1);
		}

		/* Main Run Loop */
		if (server_app_api->run_loop() != CORE_STATUS_OK)
			Log(LOG_ALERT, "Fatal: Main run loop failed!");
		if (server_app_api->shutdown_runtime() != CORE_STATUS_OK)
			Log(LOG_ALERT, "Fatal: Could not shut down runtime cleanly!");
		if (server_app_api->shutdown_runtime_loop() != CORE_STATUS_OK)
			Log(LOG_ALERT, "Fatal: Could not shut down transport cleanly!");
	}
	(void)server_app_api->stop();
	(void)server_app_api->delete_pidfile();

	return 0;
} /* main */


/**
 * Generate ngIRCd "version strings".
 *
 * The ngIRCd version information is generated once and then stored in the
 * NGIRCd_Version and NGIRCd_VersionAddition string variables for further
 * usage, for example by the IRC command "VERSION" and the --version command
 * line switch.
 */
static void
Fill_Version(void)
{
	NGIRCd_VersionAddition[0] = '\0';

#ifdef ICONV
	if (NGIRCd_VersionAddition[0])
		strlcat(NGIRCd_VersionAddition, "+",
			sizeof NGIRCd_VersionAddition);
	strlcat(NGIRCd_VersionAddition, "CHARCONV",
		sizeof NGIRCd_VersionAddition);
#endif
#ifdef DEBUG
	if (NGIRCd_VersionAddition[0])
		strlcat(NGIRCd_VersionAddition, "+",
			sizeof NGIRCd_VersionAddition);
	strlcat(NGIRCd_VersionAddition, "DEBUG",
		sizeof NGIRCd_VersionAddition);
#endif
#ifdef IDENTAUTH
	if (NGIRCd_VersionAddition[0])
		strlcat(NGIRCd_VersionAddition, "+",
			sizeof NGIRCd_VersionAddition);
	strlcat(NGIRCd_VersionAddition, "IDENT",
		sizeof NGIRCd_VersionAddition);
#endif
#ifdef WANT_IPV6
	if (NGIRCd_VersionAddition[0])
		strlcat(NGIRCd_VersionAddition, "+",
			sizeof(NGIRCd_VersionAddition));
	strlcat(NGIRCd_VersionAddition, "IPv6",
		sizeof(NGIRCd_VersionAddition));
#endif
#ifdef IRCPLUS
	if (NGIRCd_VersionAddition[0])
		strlcat(NGIRCd_VersionAddition, "+",
			sizeof NGIRCd_VersionAddition);
	strlcat(NGIRCd_VersionAddition, "IRCPLUS",
		sizeof NGIRCd_VersionAddition);
#endif
#ifdef PAM
	if (NGIRCd_VersionAddition[0])
		strlcat(NGIRCd_VersionAddition, "+",
			sizeof NGIRCd_VersionAddition);
	strlcat(NGIRCd_VersionAddition, "PAM",
		sizeof NGIRCd_VersionAddition);
#endif
#ifdef STRICT_RFC
	if (NGIRCd_VersionAddition[0])
		strlcat(NGIRCd_VersionAddition, "+",
			sizeof NGIRCd_VersionAddition);
	strlcat(NGIRCd_VersionAddition, "RFC",
		sizeof NGIRCd_VersionAddition);
#endif
#ifdef SNIFFER
	if (NGIRCd_VersionAddition[0])
		strlcat(NGIRCd_VersionAddition, "+",
			sizeof NGIRCd_VersionAddition);
	strlcat(NGIRCd_VersionAddition, "SNIFFER",
		sizeof NGIRCd_VersionAddition);
#endif
#ifdef SSL_SUPPORT
	if (NGIRCd_VersionAddition[0])
		strlcat(NGIRCd_VersionAddition, "+",
			sizeof NGIRCd_VersionAddition);
	strlcat(NGIRCd_VersionAddition, "SSL",
		sizeof NGIRCd_VersionAddition);
#endif
#ifdef SYSLOG
	if (NGIRCd_VersionAddition[0])
		strlcat(NGIRCd_VersionAddition, "+",
			sizeof NGIRCd_VersionAddition);
	strlcat(NGIRCd_VersionAddition, "SYSLOG",
		sizeof NGIRCd_VersionAddition);
#endif
#ifdef TCPWRAP
	if (NGIRCd_VersionAddition[0])
		strlcat(NGIRCd_VersionAddition, "+",
			sizeof NGIRCd_VersionAddition);
	strlcat(NGIRCd_VersionAddition, "TCPWRAP",
		sizeof NGIRCd_VersionAddition);
#endif
#ifdef ZLIB
	if (NGIRCd_VersionAddition[0])
		strlcat(NGIRCd_VersionAddition, "+",
			sizeof NGIRCd_VersionAddition);
	strlcat(NGIRCd_VersionAddition, "ZLIB",
		sizeof NGIRCd_VersionAddition);
#endif
	if (NGIRCd_VersionAddition[0])
		strlcat(NGIRCd_VersionAddition, "-",
			sizeof(NGIRCd_VersionAddition));

	strlcat(NGIRCd_VersionAddition, HOST_CPU,
		sizeof(NGIRCd_VersionAddition));
	strlcat(NGIRCd_VersionAddition, "/", sizeof(NGIRCd_VersionAddition));
	strlcat(NGIRCd_VersionAddition, HOST_VENDOR,
		sizeof(NGIRCd_VersionAddition));
	strlcat(NGIRCd_VersionAddition, "/", sizeof(NGIRCd_VersionAddition));
	strlcat(NGIRCd_VersionAddition, HOST_OS,
		sizeof(NGIRCd_VersionAddition));

	snprintf(NGIRCd_Version, sizeof NGIRCd_Version, "%s %s-%s",
		 PACKAGE_NAME, PACKAGE_VERSION, NGIRCd_VersionAddition);
} /* Fill_Version */


/**
 * Display copyright and version information of ngIRCd on the console.
 */
static void
Show_Version( void )
{
	puts( NGIRCd_Version );
	puts( "Copyright (c)2001-2024 Alexander Barton (<alex@barton.de>) and Contributors." );
	puts( "Homepage: <http://ngircd.barton.de/>\n" );
	puts( "This is free software; see the source for copying conditions. There is NO" );
	puts( "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." );
} /* Show_Version */


/**
 * Display a short help text on the console.
 * This help depends on the configuration of the executable and only shows
 * options that are actually enabled.
 */
static void
Show_Help( void )
{
	puts( "  -d, --debug        log extra debug messages" );
	puts( "  -f, --config <f>   use file <f> as configuration file" );
	puts( "  -n, --nodaemon     don't fork and don't detach from controlling terminal" );
	puts( "  -p, --passive      disable automatic connections to other servers" );
#ifdef SNIFFER
	puts( "  -s, --sniffer      enable network sniffer and display all IRC traffic" );
#endif
	puts( "  -t, --configtest   read, validate and display configuration; then exit" );
	puts( "  -V, --version      output version information and exit" );
#ifdef SYSLOG
	puts( "  -y, --syslog       log to syslog even when running in the foreground (-n)" );
#endif
	puts( "  -h, --help         display this help and exit" );
} /* Show_Help */


/* Setup_FDStreams and NGIRCd_getNobodyID moved to src/ngircd_impl/ngircd_daemon.c */

#if 0  /* moved */
static bool
NGIRCd_getNobodyID(uid_t *uid, gid_t *gid )
{
	struct passwd *pwd;

#ifdef __CYGWIN__
	/* Cygwin kludge.
	 * It can return EINVAL instead of EPERM
	 * so, if we are already unprivileged,
	 * use id of current user.
	 */
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
} /* NGIRCd_getNobodyID - stub, not compiled */

#endif /* moved */

/* NGIRCd_Init moved to src/ngircd_impl/ngircd_daemon.c */
#if 0
GLOBAL bool
NGIRCd_Init(bool NGIRCd_NoDaemon)
{
	static bool initialized;
	bool chrooted = false;
	struct passwd *pwd;
	struct group *grp;
	int real_errno, fd = -1;
	pid_t pid;
	const config_api_t *config_api = config_get_api_v1();
	config_bootstrap_t bootstrap;
	const char *chroot_dir = NULL;
	uid_t effective_uid;
	gid_t effective_gid;

	LogDebug("NGIRCd_Init(%d): starting.", NGIRCd_NoDaemon);

	if (initialized)
		return true;

	if (!NGIRCd_NoDaemon) {
		/* open /dev/null before chroot() */
		fd = open( "/dev/null", O_RDWR);
		if (fd < 0)
			Log(LOG_WARNING, "Could not open /dev/null: %s",
			    strerror(errno));
	}

#ifdef SSL_SUPPORT
	/* SSL initialization */
	if (!ConnSSL_InitLibrary()) {
		Log(LOG_ERR, "Error during SSL initialization!");
		goto out;
	}
#endif

	/* Change root */
	if (config_api && config_api->get_bootstrap &&
	    config_api->get_bootstrap(&bootstrap) == CORE_STATUS_OK &&
	    bootstrap.chroot_dir[0]) {
		chroot_dir = bootstrap.chroot_dir;

		if (chdir(chroot_dir) != 0) {
			Log(LOG_ERR, "Can't chdir() in ChrootDir (%s): %s!",
			    chroot_dir, strerror(errno));
			goto out;
		}

		if (chroot(chroot_dir) != 0) {
			Log(LOG_ERR,
			    "Can't change root directory to \"%s\": %s!",
			    chroot_dir, strerror(errno));
			goto out;
		} else {
			chrooted = true;
			Log(LOG_INFO,
			    "Changed root and working directory to \"%s\".",
			    chroot_dir);
		}
	}

#if !defined(SINGLE_USER_OS)
	/* Check user ID */
	if (config_api && config_api->get_bootstrap &&
	    config_api->get_bootstrap(&bootstrap) == CORE_STATUS_OK &&
	    bootstrap.uid == 0) {
		uid_t nobody_uid = 0;
		gid_t nobody_gid = 0;

		pwd = getpwuid(0);
		Log(LOG_INFO,
		    "ServerUID must not be %s(0), using \"nobody\" instead.",
		    pwd ? pwd->pw_name : "?");
		if (!NGIRCd_getNobodyID(&nobody_uid, &nobody_gid)) {
			Log(LOG_WARNING,
			    "Could not get user/group ID of user \"nobody\": %s",
			    errno ? strerror(errno) : "not found" );
			goto out;
		}
		bootstrap.uid = nobody_uid;
		bootstrap.gid = nobody_gid;
	}

	if (config_api && config_api->get_bootstrap &&
	    config_api->get_bootstrap(&bootstrap) == CORE_STATUS_OK) {
		effective_uid = bootstrap.uid;
		effective_gid = bootstrap.gid;
	} else {
		effective_uid = getuid();
		effective_gid = getgid();
	}

	/* Change group ID */
	if (config_api && config_api->get_bootstrap &&
	    config_api->get_bootstrap(&bootstrap) == CORE_STATUS_OK &&
	    getgid() != bootstrap.gid) {
		const gid_t gid = bootstrap.gid;

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
#endif

	/* Change user ID */
	if (config_api && config_api->get_bootstrap &&
	    config_api->get_bootstrap(&bootstrap) == CORE_STATUS_OK &&
	    getuid() != bootstrap.uid) {
		const uid_t uid = bootstrap.uid;

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

	initialized = true;

	/* Normally a child process is forked which isn't any longer
	 * connected to the controlling terminal. Use "--nodaemon"
	 * to disable this "daemon mode" (useful for debugging). */
	if (!NGIRCd_NoDaemon) {
		pid = fork();
		if (pid > 0) {
			/* "Old" process: exit. */
			exit(0);
		}
		if (pid < 0) {
			/* Error!? */
			fprintf(stderr,
				"%s: Can't fork: %s!\nFatal error, exiting now ...\n",
				PACKAGE_NAME, strerror(errno));
			exit(1);
		}

		/* New child process */
#ifdef HAVE_SETSID
		(void)setsid();
#else
		setpgrp(0, getpid());
#endif
		if (chdir("/") != 0)
			Log(LOG_ERR, "Can't change directory to '/': %s!",
				     strerror(errno));

		/* Detach stdin, stdout and stderr */
		Setup_FDStreams(fd);
		if (fd > 2)
			close(fd);
	}
	pid = getpid();

	/* Check UID/GID we are running as, can be different from values
	 * configured (e. g. if we were already started with a UID>0. */
	effective_uid = getuid();
	effective_gid = getgid();

	pwd = getpwuid(effective_uid);
	grp = getgrgid(effective_gid);

	Log(LOG_INFO, "Running as user %s(%ld), group %s(%ld), with PID %ld.",
	    pwd ? pwd->pw_name : "unknown", (long)effective_uid,
	    grp ? grp->gr_name : "unknown", (long)effective_gid, (long)pid);

	if (chrooted) {
		Log(LOG_INFO, "Running with root directory \"%s\".",
		    chroot_dir ? chroot_dir : "");
		return true;
	} else
		Log(LOG_INFO, "Not running with changed root directory.");

	/* Change working directory to home directory of the user we are
	 * running as (only when running in daemon mode and not in chroot) */

	if (NGIRCd_NoDaemon)
		return true;

	if (pwd) {
		if (chdir(pwd->pw_dir) == 0)
			LogDebug(
			    "Changed working directory to \"%s\" ...",
			    pwd->pw_dir);
		else
			Log(LOG_ERR,
			    "Can't change working directory to \"%s\": %s!",
			    pwd->pw_dir, strerror(errno));
	} else
		Log(LOG_ERR, "Can't get user information for UID %d!?", (int)effective_uid);

	return true;
 out:
	if (fd > 2)
		close(fd);
	return false;
} /* NGIRCd_Init - stub, not compiled */
#endif /* moved to ngircd_daemon.c */

/* -eof- */
