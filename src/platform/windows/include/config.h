/*
 * ngIRCd -- Windows build configuration
 * Auto-generated config.h for MSVC/Windows builds
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

/* Package information */
#define PACKAGE_NAME "ngIRCd"
#define PACKAGE "ngircd"
#define PACKAGE_VERSION "27-win32"
#define PACKAGE_STRING "ngIRCd 27-win32"
#define PACKAGE_BUGREPORT "ngircd@lists.barton.de"
#define PACKAGE_URL "https://ngircd.barton.de/"

/* Target system */
#define HOST_OS "windows"
#define HOST_CPU "x86_64"
#define HOST_VENDOR "pc"

/* Paths */
#define SYSCONFDIR "."
#define DOCDIR "."

/* Compiler features */
#define PROTOTYPES 1

/* Available headers */
#define HAVE_STDDEF_H 1
#define HAVE_STDINT_H 1
#define HAVE_STDBOOL_H 1
#define HAVE_INTTYPES_H 1
#define HAVE_ARPA_INET_H 1
#define STDC_HEADERS 1

/* Available functions */
#define HAVE_SNPRINTF 1
#define HAVE_VSNPRINTF 1
#define HAVE_STRDUP 1
#define HAVE_STRTOK_R 1
/* strlcpy/strlcat NOT available on MSVC - provided by ngportab */
/* #undef HAVE_STRLCPY */
/* #undef HAVE_STRLCAT */
#define HAVE_INET_NTOA 1
#define HAVE_MEMMOVE 1
#define HAVE_MEMSET 1
#define HAVE_STRCHR 1
#define HAVE_STRERROR 1
#define HAVE_STRSTR 1
#define HAVE_WORKING_GETADDRINFO 1
#define HAVE_GAI_STRERROR 1
#define HAVE_GETNAMEINFO 1

/* Available types */
#define HAVE_socklen_t 1

/* I/O backend: select() via Winsock */
#define HAVE_SELECT 1

/* Enabled features */
#define IRCPLUS 1

/* IPv6 support */
#define WANT_IPV6 1

/* Disabled Unix-specific features */
/* #undef SYSLOG */
/* #undef PAM */
/* #undef TCPWRAP */
/* #undef IDENTAUTH */
/* #undef ZLIB */
/* #undef SSL_SUPPORT */
/* #undef HAVE_LIBSSL */
/* #undef ICONV */
/* #undef DEBUG */
/* #undef SNIFFER */
/* #undef STRICT_RFC */

/* Unix features not available on Windows */
/* #undef HAVE_SIGACTION */
/* #undef HAVE_SIGPROCMASK */
/* #undef HAVE_SETSID */
/* #undef HAVE_SETGROUPS */
/* #undef HAVE_ARC4RANDOM */
/* #undef HAVE_ARC4RANDOM_STIR */
/* #undef HAVE_STRSIGNAL */
/* #undef HAVE_WAITPID */
/* #undef HAVE_SETRLIMIT */
/* #undef HAVE_SYS_RESOURCE_H */
/* #undef HAVE_NETINET_IP_H */
/* #undef HAVE_NETINET_IN_SYSTM_H */
/* #undef HAVE_SYS_UN_H */
/* #undef HAVE_POLL */
/* #undef HAVE_POLL_H */
/* #undef HAVE_EPOLL_CREATE */
/* #undef HAVE_KQUEUE */
/* #undef HAVE_SYS_DEVPOLL_H */
/* #undef HAVE_MTRACE */
/* #undef HAVE_LONG_LONG */

/* Windows is a "single user OS" in the ngIRCd sense (no uid/gid switching) */
#define SINGLE_USER_OS 1

/* --- Windows-specific compatibility --- */
#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

/* Suppress MSVC deprecation warnings */
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

/* MSVC-specific pragmas */
#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif

/* Type definitions - define these before anything else */
typedef int pid_t;
typedef long long ssize_t;
typedef int uid_t;
typedef int gid_t;

/* String function mappings */
#define strcasecmp _stricmp
#define strncasecmp _strnicmp

/* strtok_r -> strtok_s on MSVC */
#ifndef strtok_r
#define strtok_r(s, d, p) strtok_s(s, d, p)
#endif

#endif /* _WIN32 */

#endif /* __CONFIG_H__ */
