/*
 * ngIRCd -- Windows Compatibility Layer
 * Provides Unix API shims for building on Windows with MSVC.
 * Include this AFTER portab.h / config.h in .c files that need socket or process APIs.
 */

#ifndef __WIN32COMPAT_H__
#define __WIN32COMPAT_H__

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef FD_SETSIZE
#define FD_SETSIZE 1024
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <io.h>
#include <process.h>
#include <direct.h>
#include <fcntl.h>

/* --- Missing POSIX constants --- */

#ifndef S_IRUSR
#define S_IRUSR _S_IREAD
#endif
#ifndef S_IWUSR
#define S_IWUSR _S_IWRITE
#endif
#ifndef S_IRGRP
#define S_IRGRP 0
#endif
#ifndef S_IROTH
#define S_IROTH 0
#endif

#ifndef O_NONBLOCK
#define O_NONBLOCK 0
#endif

/* Signal constants */
#ifndef SIGQUIT
#define SIGQUIT SIGBREAK
#endif
#ifndef SIGHUP
#define SIGHUP SIGBREAK
#endif
#ifndef SIGCHLD
#define SIGCHLD 0
#endif
#ifndef SIGUSR1
#define SIGUSR1 0
#endif
#ifndef SIGUSR2
#define SIGUSR2 0
#endif
#ifndef SIGPIPE
#define SIGPIPE 0
#endif
#ifndef SIGALRM
#define SIGALRM 0
#endif

#ifndef WNOHANG
#define WNOHANG 1
#endif

#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif
#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif
#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

#ifndef PF_INET
#define PF_INET AF_INET
#endif
#ifndef PF_INET6
#define PF_INET6 AF_INET6
#endif
#ifndef PF_UNSPEC
#define PF_UNSPEC AF_UNSPEC
#endif

#ifndef NI_MAXHOST
#define NI_MAXHOST 1025
#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#ifndef SOCK_CLOEXEC
#define SOCK_CLOEXEC 0
#endif

/* --- Function mappings --- */

#define getpid() _getpid()
#define umask(m) _umask(m)

/* File descriptor functions */
#ifndef _WIN32_FD_COMPAT
#define _WIN32_FD_COMPAT
#define open _open

/* close(): try closesocket() first; fall back to _close() for plain fds */
static __inline int _win32_smart_close(int fd) {
	if (closesocket((SOCKET)fd) == 0) return 0;
	if (WSAGetLastError() == WSAENOTSOCK) return _close(fd);
	errno = EBADF; return -1;
}
#define close _win32_smart_close

/* read(): try recv() first; fall back to _read() for plain fds */
static __inline int _win32_smart_read(int fd, void *buf, unsigned int len) {
	int r = recv((SOCKET)fd, (char *)buf, (int)len, 0);
	if (r != SOCKET_ERROR) return r;
	if (WSAGetLastError() == WSAENOTSOCK) return _read(fd, buf, len);
	errno = EIO; return -1;
}
#define read(fd, buf, len) _win32_smart_read(fd, buf, len)

#define write _write
#define dup2 _dup2
#define lseek _lseek
#define unlink _unlink
#define chdir _chdir
#define getcwd _getcwd
#define access _access
#define mkdir(d, m) _mkdir(d)
#define pipe(fds) _pipe(fds, 4096, _O_BINARY)
#define isatty _isatty
#endif

/* setpgrp is a no-op on Windows */
#define setpgrp(...) (0)

/* Process stubs */
static __inline pid_t fork(void) { return -1; }
static __inline int setuid(uid_t u) { (void)u; return 0; }
static __inline int setgid(gid_t g) { (void)g; return 0; }
static __inline uid_t getuid(void) { return 1; }
static __inline gid_t getgid(void) { return 1; }
static __inline uid_t geteuid(void) { return 1; }
static __inline pid_t waitpid(pid_t p, int *s, int o) { (void)p; (void)s; (void)o; return -1; }
static __inline int setsid(void) { return 0; }
static __inline unsigned int alarm(unsigned int s) { (void)s; return 0; }
static __inline int chroot(const char *p) { (void)p; return -1; }

/* fcntl replacement */
#define F_GETFL 3
#define F_SETFL 4
#define F_GETFD 1
#define F_SETFD 2
#ifndef FD_CLOEXEC
#define FD_CLOEXEC 1
#endif

static __inline int fcntl(int fd, int cmd, ...) {
	if (cmd == F_GETFL || cmd == F_GETFD) return 0;
	if (cmd == F_SETFL) {
		u_long mode = 1;
		ioctlsocket((SOCKET)fd, FIONBIO, &mode);
		return 0;
	}
	if (cmd == F_SETFD) return 0;
	return -1;
}

/* gettimeofday replacement */
static __inline int gettimeofday(struct timeval *tv, void *tz) {
	FILETIME ft;
	ULARGE_INTEGER uli;
	(void)tz;
	GetSystemTimeAsFileTime(&ft);
	uli.LowPart = ft.dwLowDateTime;
	uli.HighPart = ft.dwHighDateTime;
	/* Convert from 100ns intervals since 1601 to microseconds since 1970 */
	uli.QuadPart -= 116444736000000000ULL;
	tv->tv_sec = (long)(uli.QuadPart / 10000000ULL);
	tv->tv_usec = (long)((uli.QuadPart % 10000000ULL) / 10);
	return 0;
}

#define sleep(s) Sleep((s) * 1000)

#endif /* _WIN32 */
#endif /* __WIN32COMPAT_H__ */
