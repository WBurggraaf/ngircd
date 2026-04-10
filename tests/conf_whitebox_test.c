#include "portab.h"
#include "conn.h"
#include "conf.h"
#include "module_iface.h"

#include <stdio.h>
#include <string.h>

static int log_calls;
static int warn_calls;

static void
test_log_init(bool syslog_mode)
{
	(void)syslog_mode;
}

static void
test_log_exit(void)
{
}

static void
test_log_reinit(void)
{
}

static void
test_log(int level, const char *fmt, ...)
{
	(void)level;
	(void)fmt;
	log_calls++;
}

static void
test_debug(const char *fmt, ...)
{
	(void)fmt;
	warn_calls++;
}

static const NgLogOps log_ops = {
	test_log_init,
	test_log_exit,
	test_log_reinit,
	test_log,
	test_debug
};

static int
check(bool cond, const char *msg)
{
	if (!cond) {
		fprintf(stderr, "%s\n", msg);
		return 1;
	}
	return 0;
}

int
main(void)
{
	int rc = 0;
	char var[64];
	char arg[128];

	NgLog_SetOps(&log_ops);

	Conf_Test_SetDefaults(true);
	rc |= check(Conf_AllowedChannelTypes[0] != '\0', "defaults not set");

	strcpy(var, "Name");
	strcpy(arg, "whitebox.server");
	Conf_Test_Handle_GLOBAL("test.conf", 1, var, arg);
	rc |= check(strcmp(Conf_ServerName, "whitebox.server") == 0, "global handler failed");

	strcpy(var, "Ports");
	strcpy(arg, "7000,7001");
	Conf_Test_Handle_GLOBAL("test.conf", 2, var, arg);

	strcpy(var, "ConnectIPv6");
	strcpy(arg, "no");
	Conf_Test_Handle_OPTIONS("test.conf", 3, var, arg);
	rc |= check(!Conf_ConnectIPv6, "options handler failed");

	strcpy(var, "MaxConnections");
	strcpy(arg, "42");
	Conf_Test_Handle_LIMITS("test.conf", 4, var, arg);
	(void)Conf_MaxConnections;

	strcpy(var, "Name");
	strcpy(arg, "oper");
	Conf_Test_Handle_OPERATOR("test.conf", 5, var, arg);
	strcpy(var, "Name");
	strcpy(arg, "server.example.org");
	Conf_Test_Handle_SERVER("test.conf", 6, var, arg);

	strcpy(var, "Name");
	strcpy(arg, "#whitebox");
	Conf_Test_Handle_CHANNEL("test.conf", 7, var, arg);

	(void)Conf_Test_ValidateConfig(true, false);

	(void)log_calls;
	(void)warn_calls;
	return rc;
}
