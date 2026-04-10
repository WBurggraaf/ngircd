#include "portab.h"
#include "proc.h"
#include <sys/time.h>
#include "log.h"
#include "io.h"
#include "module_iface.h"

#include <stdio.h>
#include <string.h>

static int log_init_calls;
static int log_exit_calls;
static int log_reinit_calls;
static int io_init_calls;
static int io_shutdown_calls;
static int io_dispatch_calls;
static int resolver_calls;

static void test_log_init(bool syslog_mode) { (void)syslog_mode; log_init_calls++; }
static void test_log_exit(void) { log_exit_calls++; }
static void test_log_reinit(void) { log_reinit_calls++; }
static void test_log(int level, const char *fmt, ...) { (void)level; (void)fmt; }
static void test_debug(const char *fmt, ...) { (void)fmt; }

static bool test_io_init(unsigned int event_capacity) { (void)event_capacity; io_init_calls++; return true; }
static void test_io_shutdown(void) { io_shutdown_calls++; }
static int test_io_dispatch(struct timeval *tv) { (void)tv; io_dispatch_calls++; return 7; }

static bool test_resolve_addr(PROC_STAT *procstat, const void *addr, int identsock, void (*cbfunc)(int, short))
{ (void)procstat; (void)addr; (void)identsock; (void)cbfunc; resolver_calls++; return true; }
static bool test_resolve_name(PROC_STAT *procstat, const char *host, void (*cbfunc)(int, short))
{ (void)procstat; (void)host; (void)cbfunc; resolver_calls++; return true; }

static const NgLogOps log_ops = { test_log_init, test_log_exit, test_log_reinit, test_log, test_debug };
static const NgIoOps io_ops = { test_io_init, test_io_shutdown, NULL, NULL, NULL, NULL, NULL, test_io_dispatch };
static const NgResolverOps resolver_ops = { test_resolve_addr, test_resolve_name };

static int fail(const char *msg) { fprintf(stderr, "%s\n", msg); return 1; }

int main(void)
{
	NgLog_SetOps(&log_ops);
	NgIo_SetOps(&io_ops);
	NgResolver_SetOps(&resolver_ops);

	Log_Init(false);
	Log_ReInit();
	Log_Exit();

	if (log_init_calls != 1 || log_reinit_calls != 1 || log_exit_calls != 1)
		return fail("log ops not invoked");

	if (!io_library_init(32))
		return fail("io init failed");
	io_library_shutdown();
	if (io_dispatch(NULL) != 7)
		return fail("io dispatch failed");

	if (io_init_calls != 1 || io_shutdown_calls != 1 || io_dispatch_calls != 1)
		return fail("io ops not invoked");

	if (NgResolver_GetOps()->resolve_name == NULL || NgResolver_GetOps()->resolve_addr_ident == NULL)
		return fail("resolver ops missing");
	NgResolver_GetOps()->resolve_name(NULL, "example.org", NULL);
	NgResolver_GetOps()->resolve_addr_ident(NULL, NULL, 0, NULL);
	if (resolver_calls < 2)
		return fail("resolver ops not invoked");

	NgLog_SetOps(NULL);
	NgIo_SetOps(NULL);
	NgResolver_SetOps(NULL);
	return 0;
}
