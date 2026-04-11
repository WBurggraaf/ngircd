/*
 * ngIRCd -- JOIN access decision helper
 */

#ifndef __join_check_h__
#define __join_check_h__

#include "portab.h"

typedef enum {
	JOIN_CHECK_OK = 0,
	JOIN_CHECK_BANNED,
	JOIN_CHECK_INVITEONLY,
	JOIN_CHECK_BADKEY,
	JOIN_CHECK_FULL,
	JOIN_CHECK_SECUREONLY,
	JOIN_CHECK_OPONLY,
	JOIN_CHECK_REGONLY
} JOIN_CHECK_RESULT;

GLOBAL JOIN_CHECK_RESULT Join_Check_Access PARAMS((bool is_operator,
						   bool is_banned,
						   bool is_invited,
						   bool is_exception,
						   bool key_ok,
						   bool is_full,
						   bool secure_required,
						   bool has_ssl,
						   bool oper_only,
						   bool reg_only,
						   bool is_registered));

#endif
