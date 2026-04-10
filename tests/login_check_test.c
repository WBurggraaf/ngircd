#include "portab.h"
#include "login_check.h"

#include <stdio.h>

static int fail(const char *msg) { fprintf(stderr, "%s\n", msg); return 1; }

int main(void)
{
	if (Login_Check_Pass(LOGIN_CLIENT_UNKNOWN, 1) != LOGIN_CHECK_ALLOWED)
		return fail("pass rfc1459");
	if (Login_Check_Pass(LOGIN_CLIENT_UNKNOWN, 3) != LOGIN_CHECK_ALLOWED)
		return fail("pass rfc2813");
	if (Login_Check_Pass(LOGIN_CLIENT_UNKNOWN, 2) != LOGIN_CHECK_NEEDMOREPARAMS)
		return fail("pass missing params");
	if (Login_Check_Pass(LOGIN_CLIENT_USER, 1) != LOGIN_CHECK_ALREADYREGISTERED)
		return fail("pass registered");
	if (Login_Check_Pass(LOGIN_CLIENT_UNKNOWNSERVER, 2) != LOGIN_CHECK_NEEDMOREPARAMS)
		return fail("pass unknownserver missing");
	if (Login_Check_Pass(LOGIN_CLIENT_UNKNOWNSERVER, 4) != LOGIN_CHECK_ALLOWED)
		return fail("pass unknownserver allowed");
	return 0;
}
