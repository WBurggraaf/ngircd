#include "portab.h"
#include "join_error.h"

#include <stdio.h>

static int fail(const char *msg) { fprintf(stderr, "%s\n", msg); return 1; }

int main(void)
{
	if (Join_Error_FromCheck(JOIN_CHECK_OK) != JOIN_ERROR_NONE)
		return fail("join none");
	if (Join_Error_FromCheck(JOIN_CHECK_BANNED) != JOIN_ERROR_BANNED)
		return fail("join banned");
	if (Join_Error_FromCheck(JOIN_CHECK_INVITEONLY) != JOIN_ERROR_INVITEONLY)
		return fail("join invite");
	if (Join_Error_FromCheck(JOIN_CHECK_BADKEY) != JOIN_ERROR_BADKEY)
		return fail("join key");
	if (Join_Error_FromCheck(JOIN_CHECK_FULL) != JOIN_ERROR_FULL)
		return fail("join full");
	if (Join_Error_FromCheck(JOIN_CHECK_SECUREONLY) != JOIN_ERROR_SECUREONLY)
		return fail("join secure");
	if (Join_Error_FromCheck(JOIN_CHECK_OPONLY) != JOIN_ERROR_OPONLY)
		return fail("join oper");
	if (Join_Error_FromCheck(JOIN_CHECK_REGONLY) != JOIN_ERROR_REGONLY)
		return fail("join reg");
	return 0;
}
