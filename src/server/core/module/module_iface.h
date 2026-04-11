/*
 * ngIRCd module interface seams for tests and stubs.
 *
 * These tables describe the major runtime modules so unit tests can
 * substitute behavior without patching the production code paths.
 */

#ifndef NGIRCD_MODULE_IFACE_H
#define NGIRCD_MODULE_IFACE_H

#include "portab.h"
#include "proc.h"

typedef struct NgPlatformOps {
	int (*open_file)(const char *path, int flags, ...);
	int (*close_file)(int fd);
	int (*read_file)(int fd, void *buf, size_t len);
	int (*write_file)(int fd, const void *buf, size_t len);
	int (*dup_file)(int src, int dst);
	int (*pipe_open)(int fds[2]);
	int (*set_nonblock)(int fd);
	int (*set_cloexec)(int fd);
	unsigned int (*get_pid)(void);
	unsigned int (*set_umask)(unsigned int mask);
} NgPlatformOps;

typedef struct NgLogOps {
	void (*init)(bool syslog_mode);
	void (*exit)(void);
	void (*reinit)(void);
	void (*log)(int level, const char *message);
	void (*debug)(const char *message);
} NgLogOps;

typedef struct NgIoOps {
	bool (*library_init)(unsigned int event_capacity);
	void (*library_shutdown)(void);
	bool (*event_create)(int fd, short what, void (*cbfunc)(int, short));
	bool (*event_setcb)(int fd, void (*cbfunc)(int, short));
	bool (*event_add)(int fd, short what);
	bool (*event_del)(int fd, short what);
	bool (*close_fd)(int fd);
	int (*dispatch)(struct timeval *tv);
} NgIoOps;

typedef struct NgResolverOps {
	bool (*resolve_addr_ident)(PROC_STAT *procstat, const void *addr,
		int identsock, void (*cbfunc)(int, short));
	bool (*resolve_name)(PROC_STAT *procstat, const char *host,
		void (*cbfunc)(int, short));
} NgResolverOps;

typedef struct NgSignalOps {
	bool (*init)(void);
	void (*exit)(void);
	bool (*notify_possible)(void);
	void (*notify_service_manager)(const char *message);
} NgSignalOps;

GLOBAL void NgPlatform_SetOps PARAMS((const NgPlatformOps *Ops));
GLOBAL const NgPlatformOps *NgPlatform_GetOps PARAMS((void));

GLOBAL void NgLog_SetOps PARAMS((const NgLogOps *Ops));
GLOBAL const NgLogOps *NgLog_GetOps PARAMS((void));

GLOBAL void NgIo_SetOps PARAMS((const NgIoOps *Ops));
GLOBAL const NgIoOps *NgIo_GetOps PARAMS((void));

GLOBAL void NgResolver_SetOps PARAMS((const NgResolverOps *Ops));
GLOBAL const NgResolverOps *NgResolver_GetOps PARAMS((void));

GLOBAL void NgSignal_SetOps PARAMS((const NgSignalOps *Ops));
GLOBAL const NgSignalOps *NgSignal_GetOps PARAMS((void));

#endif
