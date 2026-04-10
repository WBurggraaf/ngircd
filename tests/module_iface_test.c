#include "portab.h"
#include "module_iface.h"

#include <stdio.h>

static int log_init_calls;
static int log_exit_calls;
static int io_init_calls;
static int io_shutdown_calls;

static void
test_log_init(bool syslog_mode)
{
	(void)syslog_mode;
	log_init_calls++;
}

static void
test_log_exit(void)
{
	log_exit_calls++;
}

static bool
test_io_init(unsigned int event_capacity)
{
	(void)event_capacity;
	io_init_calls++;
	return true;
}

static void
test_io_shutdown(void)
{
	io_shutdown_calls++;
}

static const NgLogOps log_ops = {
	test_log_init,
	test_log_exit,
	NULL,
	NULL,
	NULL
};

static const NgIoOps io_ops = {
	test_io_init,
	test_io_shutdown,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

int
main(void)
{
	if (NgLog_GetOps() != NULL || NgIo_GetOps() != NULL) {
		fprintf(stderr, "expected empty registry\n");
		return 1;
	}

	NgLog_SetOps(&log_ops);
	NgIo_SetOps(&io_ops);

	if (NgLog_GetOps() != &log_ops || NgIo_GetOps() != &io_ops) {
		fprintf(stderr, "registry round-trip failed\n");
		return 1;
	}

	if (NgLog_GetOps()->init == NULL || NgIo_GetOps()->library_init == NULL) {
		fprintf(stderr, "ops table incomplete\n");
		return 1;
	}

	NgLog_GetOps()->init(false);
	NgLog_GetOps()->exit();
	NgIo_GetOps()->library_init(16);
	NgIo_GetOps()->library_shutdown();

	if (log_init_calls != 1 || log_exit_calls != 1 ||
	    io_init_calls != 1 || io_shutdown_calls != 1) {
		fprintf(stderr, "callback dispatch failed\n");
		return 1;
	}

	NgLog_SetOps(NULL);
	NgIo_SetOps(NULL);

	return 0;
}
