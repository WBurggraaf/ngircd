#include "portab.h"
#include "join_check.h"

#include <stdio.h>

static int fail(const char *msg) { fprintf(stderr, "%s\n", msg); return 1; }

int main(void)
{
	if (Join_Check_Access(false, false, false, false, true, false, false, false, false, false, false) != JOIN_CHECK_OK)
		return fail("join ok");
	if (Join_Check_Access(false, true, false, false, true, false, false, false, false, false, false) != JOIN_CHECK_BANNED)
		return fail("join banned");
	if (Join_Check_Access(false, false, false, false, false, false, false, false, false, false, false) != JOIN_CHECK_BADKEY)
		return fail("join badkey");
	if (Join_Check_Access(false, false, false, false, true, true, false, false, false, false, false) != JOIN_CHECK_FULL)
		return fail("join full");
	if (Join_Check_Access(false, false, false, false, true, false, true, false, false, false, false) != JOIN_CHECK_SECUREONLY)
		return fail("join ssl");
	if (Join_Check_Access(false, false, false, false, true, false, false, false, true, false, false) != JOIN_CHECK_OPONLY)
		return fail("join oper");
	if (Join_Check_Access(false, false, false, false, true, false, false, false, false, true, false) != JOIN_CHECK_REGONLY)
		return fail("join reg");
	return 0;
}
