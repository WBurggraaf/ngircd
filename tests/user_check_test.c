#include "portab.h"
#include "user_check.h"

#include <stdio.h>

static int fail(const char *msg) { fprintf(stderr, "%s\n", msg); return 1; }

int main(void)
{
	if (User_Check_Context(USER_CLIENT_GOTNICK) != USER_CHECK_NEW_CONNECTION)
		return fail("gotnick new");
	if (User_Check_Context(USER_CLIENT_GOTPASS) != USER_CHECK_NEW_CONNECTION)
		return fail("gotpass new");
	if (User_Check_Context(USER_CLIENT_SERVER) != USER_CHECK_SERVER_UPDATE)
		return fail("server update");
	if (User_Check_Context(USER_CLIENT_SERVICE) != USER_CHECK_SERVER_UPDATE)
		return fail("service update");
	if (User_Check_Context(USER_CLIENT_USER) != USER_CHECK_ALREADY_REGISTERED)
		return fail("already registered");
	if (User_Check_Context(USER_CLIENT_UNKNOWN) !=
#ifdef STRICT_RFC
	    USER_CHECK_NOT_REGISTERED
#else
	    USER_CHECK_NEW_CONNECTION
#endif
	    )
		return fail("unknown (STRICT_RFC dependent)");
	return 0;
}
