/*
 * ngIRCd -- USER command classification helper
 */

#ifndef __user_check_h__
#define __user_check_h__

#include "portab.h"

/* Minimal client type bits needed by this helper. */
#define USER_CLIENT_UNKNOWN		0x0001
#define USER_CLIENT_GOTPASS		0x0002
#define USER_CLIENT_GOTNICK		0x0004
#define USER_CLIENT_GOTUSER		0x0008
#define USER_CLIENT_USER		0x0010
#define USER_CLIENT_SERVER		0x0020
#define USER_CLIENT_SERVICE		0x0040

typedef enum {
	USER_CHECK_NEW_CONNECTION = 0,
	USER_CHECK_SERVER_UPDATE,
	USER_CHECK_ALREADY_REGISTERED,
	USER_CHECK_NOT_REGISTERED
} USER_CHECK_RESULT;

GLOBAL USER_CHECK_RESULT User_Check_Context PARAMS((int client_type));

#endif
