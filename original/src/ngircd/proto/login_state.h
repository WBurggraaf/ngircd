/*
 * ngIRCd -- Login state helpers
 */

#ifndef __login_state_h__
#define __login_state_h__

#include "conn.h"
#include "client.h"

typedef enum {
	LOGIN_ACTION_NONE = 0,
	LOGIN_ACTION_REGISTER,
	LOGIN_ACTION_WAIT,
	LOGIN_ACTION_REJECT
} LOGIN_ACTION;

typedef struct {
	bool cap_pending;
	bool auth_ping_pending;
	bool has_nick;
	bool has_user;
	bool is_registered;
} LOGIN_STATE;

GLOBAL LOGIN_ACTION Login_State_Resolve PARAMS((const LOGIN_STATE *State));
GLOBAL bool Login_State_CanComplete PARAMS((const CLIENT *Client));
GLOBAL bool Login_State_ShouldLoginNow PARAMS((int client_type, bool got_nick,
					       bool got_user, bool cap_pending,
					       bool auth_ping_pending));

#endif
