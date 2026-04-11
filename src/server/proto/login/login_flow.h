/*
 * ngIRCd -- Login flow decision helper
 */

#ifndef __login_flow_h__
#define __login_flow_h__

#include "portab.h"

/* Minimal client type bits needed by this helper. */
#define LOGIN_CLIENT_UNKNOWN		0x0001
#define LOGIN_CLIENT_GOTPASS		0x0002
#define LOGIN_CLIENT_GOTNICK		0x0004
#define LOGIN_CLIENT_GOTUSER		0x0008
#define LOGIN_CLIENT_USER		0x0010
#define LOGIN_CLIENT_SERVICE		0x0040

typedef enum {
	LOGIN_FLOW_KEEP_STATE = 0,
	LOGIN_FLOW_SET_GOTNICK,
	LOGIN_FLOW_SET_GOTUSER,
	LOGIN_FLOW_LOGIN_NOW
} LOGIN_FLOW_RESULT;

GLOBAL LOGIN_FLOW_RESULT Login_Flow_OnNick PARAMS((int client_type,
						   bool user_already_received,
						   bool auth_ping_pending,
						   bool cap_pending,
						   bool registration_allowed));

GLOBAL LOGIN_FLOW_RESULT Login_Flow_OnUser PARAMS((int client_type,
						   bool nick_already_received,
						   bool auth_ping_pending,
						   bool cap_pending,
						   bool registration_allowed));

#endif
