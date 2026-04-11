#include "portab.h"

#include "join_error.h"

GLOBAL JOIN_ERROR
Join_Error_FromCheck(JOIN_CHECK_RESULT Result)
{
	switch (Result) {
	case JOIN_CHECK_BANNED:
		return JOIN_ERROR_BANNED;
	case JOIN_CHECK_INVITEONLY:
		return JOIN_ERROR_INVITEONLY;
	case JOIN_CHECK_BADKEY:
		return JOIN_ERROR_BADKEY;
	case JOIN_CHECK_FULL:
		return JOIN_ERROR_FULL;
	case JOIN_CHECK_SECUREONLY:
		return JOIN_ERROR_SECUREONLY;
	case JOIN_CHECK_OPONLY:
		return JOIN_ERROR_OPONLY;
	case JOIN_CHECK_REGONLY:
		return JOIN_ERROR_REGONLY;
	case JOIN_CHECK_OK:
	default:
		return JOIN_ERROR_NONE;
	}
}
