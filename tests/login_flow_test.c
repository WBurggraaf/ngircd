#include "portab.h"
#include "login_flow.h"

#include <stdio.h>

static int fail(const char *msg) { fprintf(stderr, "%s\n", msg); return 1; }

int main(void)
{
	if (Login_Flow_OnNick(LOGIN_CLIENT_UNKNOWN, true, false, false, true) != LOGIN_FLOW_LOGIN_NOW)
		return fail("nick login now");
	if (Login_Flow_OnNick(LOGIN_CLIENT_UNKNOWN, false, false, false, true) != LOGIN_FLOW_SET_GOTNICK)
		return fail("nick set gotnick");
	if (Login_Flow_OnNick(LOGIN_CLIENT_USER, false, false, false, true) != LOGIN_FLOW_SET_GOTNICK)
		return fail("nick user state");
	if (Login_Flow_OnUser(LOGIN_CLIENT_GOTNICK, true, false, false, true) != LOGIN_FLOW_LOGIN_NOW)
		return fail("user login now");
	if (Login_Flow_OnUser(LOGIN_CLIENT_GOTNICK, false, false, false, true) != LOGIN_FLOW_SET_GOTUSER)
		return fail("user set gotuser");
	if (Login_Flow_OnUser(LOGIN_CLIENT_USER, false, false, false, true) != LOGIN_FLOW_KEEP_STATE)
		return fail("user registered keep");
	return 0;
}
