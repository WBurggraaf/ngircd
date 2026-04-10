#include "portab.h"
#include "login_state.h"
#include "client.h"

#include <stdio.h>

static int fail(const char *msg) { fprintf(stderr, "%s\n", msg); return 1; }

int main(void)
{
	LOGIN_STATE s = {0};
	if (Login_State_Resolve(NULL) != LOGIN_ACTION_REJECT)
		return fail("null state should reject");
	if (Login_State_Resolve(&s) != LOGIN_ACTION_NONE)
		return fail("empty state should do nothing");
	s.has_nick = true;
	s.has_user = true;
	if (Login_State_Resolve(&s) != LOGIN_ACTION_REGISTER)
		return fail("nick+user should register");
	s.cap_pending = true;
	if (Login_State_Resolve(&s) != LOGIN_ACTION_REGISTER)
		return fail("cap pending should not block register");
	s.is_registered = true;
	if (Login_State_Resolve(&s) != LOGIN_ACTION_NONE)
		return fail("registered should do nothing");
	if (!Login_State_ShouldLoginNow(CLIENT_UNKNOWN, true, true, false, false))
		return fail("nick+user should log in");
	if (Login_State_ShouldLoginNow(CLIENT_UNKNOWN, true, false, false, false))
		return fail("nick without user should not log in");
	return 0;
}
