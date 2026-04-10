#include "portab.h"
#include "nick_check.h"

#include <stdio.h>

static int fail(const char *msg) { fprintf(stderr, "%s\n", msg); return 1; }

int main(void)
{
	if (Nick_Check_Context(NICK_CLIENT_UNKNOWN, 1) != NICK_CHECK_USER_OR_CHANGE)
		return fail("unknown user");
	if (Nick_Check_Context(NICK_CLIENT_GOTPASS, 1) != NICK_CHECK_USER_OR_CHANGE)
		return fail("gotpass user");
	if (Nick_Check_Context(NICK_CLIENT_GOTNICK, 1) != NICK_CHECK_USER_OR_CHANGE)
		return fail("gotnick user");
	if (Nick_Check_Context(NICK_CLIENT_SERVER, 2) != NICK_CHECK_SERVER_INTRODUCE)
		return fail("server intro 2");
	if (Nick_Check_Context(NICK_CLIENT_SERVER, 7) != NICK_CHECK_SERVER_INTRODUCE)
		return fail("server intro 7");
	if (Nick_Check_Context(NICK_CLIENT_SERVER, 1) != NICK_CHECK_USER_OR_CHANGE)
		return fail("server nickchange");
	if (Nick_Check_Context(NICK_CLIENT_USER, 1) != NICK_CHECK_USER_OR_CHANGE)
		return fail("user change");
	return 0;
}
