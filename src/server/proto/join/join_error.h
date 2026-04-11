/*
 * ngIRCd -- JOIN error mapping helper
 */

#ifndef __join_error_h__
#define __join_error_h__

#include "portab.h"
#include "join_check.h"

typedef enum {
	JOIN_ERROR_NONE = 0,
	JOIN_ERROR_BANNED,
	JOIN_ERROR_INVITEONLY,
	JOIN_ERROR_BADKEY,
	JOIN_ERROR_FULL,
	JOIN_ERROR_SECUREONLY,
	JOIN_ERROR_OPONLY,
	JOIN_ERROR_REGONLY
} JOIN_ERROR;

GLOBAL JOIN_ERROR Join_Error_FromCheck PARAMS((JOIN_CHECK_RESULT Result));

#endif
