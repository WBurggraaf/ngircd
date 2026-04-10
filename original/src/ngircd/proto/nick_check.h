/*
 * ngIRCd -- NICK command classification helper
 */

#ifndef __nick_check_h__
#define __nick_check_h__

#include "portab.h"

/* Minimal client type bits needed by this helper. */
#define NICK_CLIENT_UNKNOWN		0x0001
#define NICK_CLIENT_GOTPASS		0x0002
#define NICK_CLIENT_GOTNICK		0x0004
#define NICK_CLIENT_GOTUSER		0x0008
#define NICK_CLIENT_USER		0x0010
#define NICK_CLIENT_SERVER		0x0020
#define NICK_CLIENT_SERVICE		0x0040

typedef enum {
	NICK_CHECK_USER_OR_CHANGE = 0,
	NICK_CHECK_SERVER_INTRODUCE,
	NICK_CHECK_ALREADY_REGISTERED
} NICK_CHECK_RESULT;

GLOBAL NICK_CHECK_RESULT Nick_Check_Context PARAMS((int client_type,
						     int argc));

#endif
