/*
 * ngIRCd -- Login command validation helpers
 */

#ifndef __login_check_h__
#define __login_check_h__

#include "portab.h"

/* Minimal client type bits needed by this helper. */
#define LOGIN_CLIENT_UNKNOWN		0x0001
#define LOGIN_CLIENT_UNKNOWNSERVER	0x0080
#define LOGIN_CLIENT_USER		0x0010

typedef enum {
	LOGIN_CHECK_OK = 0,
	LOGIN_CHECK_UNKNOWNCOMMAND,
	LOGIN_CHECK_NEEDMOREPARAMS,
	LOGIN_CHECK_ALREADYREGISTERED,
	LOGIN_CHECK_ALLOWED
} LOGIN_CHECK_RESULT;

GLOBAL LOGIN_CHECK_RESULT Login_Check_Pass PARAMS((int client_type, int argc));

#endif
