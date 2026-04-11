#include "portab.h"

#include "login_flow.h"

static LOGIN_FLOW_RESULT
login_flow_decide(bool primary_received, bool cap_pending,
		  bool auth_ping_pending, bool registration_allowed,
		  LOGIN_FLOW_RESULT set_state_result)
{
	if (!registration_allowed)
		return set_state_result;
	if (auth_ping_pending || cap_pending)
		return set_state_result;
	if (primary_received)
		return LOGIN_FLOW_LOGIN_NOW;
	return set_state_result;
}

GLOBAL LOGIN_FLOW_RESULT
Login_Flow_OnNick(int client_type, bool user_already_received,
		  bool cap_pending, bool auth_ping_pending,
		  bool registration_allowed)
{
	switch (client_type) {
	case LOGIN_CLIENT_UNKNOWN:
	case LOGIN_CLIENT_GOTPASS:
	case LOGIN_CLIENT_GOTNICK:
#ifndef STRICT_RFC
	case LOGIN_CLIENT_GOTUSER:
#endif
	case LOGIN_CLIENT_USER:
	case LOGIN_CLIENT_SERVICE:
		return login_flow_decide(user_already_received, cap_pending,
					 auth_ping_pending, registration_allowed,
					 LOGIN_FLOW_SET_GOTNICK);
	default:
		return LOGIN_FLOW_KEEP_STATE;
	}
}

GLOBAL LOGIN_FLOW_RESULT
Login_Flow_OnUser(int client_type, bool nick_already_received,
		  bool cap_pending, bool auth_ping_pending,
		  bool registration_allowed)
{
	switch (client_type) {
	case LOGIN_CLIENT_GOTNICK:
#ifndef STRICT_RFC
	case LOGIN_CLIENT_UNKNOWN:
#endif
	case LOGIN_CLIENT_GOTPASS:
		return login_flow_decide(nick_already_received, cap_pending,
					 auth_ping_pending, registration_allowed,
					 LOGIN_FLOW_SET_GOTUSER);
	default:
		return LOGIN_FLOW_KEEP_STATE;
	}
}
