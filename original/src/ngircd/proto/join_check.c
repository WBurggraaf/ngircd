#include "portab.h"

#include "join_check.h"

GLOBAL JOIN_CHECK_RESULT
Join_Check_Access(bool is_operator, bool is_banned, bool is_invited,
		  bool is_exception, bool key_ok, bool is_full,
		  bool secure_required, bool has_ssl, bool oper_only,
		  bool reg_only, bool is_registered)
{
	if (is_operator)
		return JOIN_CHECK_OK;
	if (is_banned && !is_invited && !is_exception)
		return JOIN_CHECK_BANNED;
	if (!key_ok)
		return JOIN_CHECK_BADKEY;
	if (is_full)
		return JOIN_CHECK_FULL;
	if (secure_required && !has_ssl)
		return JOIN_CHECK_SECUREONLY;
	if (oper_only && !is_operator)
		return JOIN_CHECK_OPONLY;
	if (reg_only && !is_registered)
		return JOIN_CHECK_REGONLY;
	return JOIN_CHECK_OK;
}
