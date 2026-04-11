#include "portab.h"

#include "login_state.h"

GLOBAL LOGIN_ACTION
Login_State_Resolve(const LOGIN_STATE *State)
{
	if (State == NULL)
		return LOGIN_ACTION_REJECT;
	if (State->is_registered)
		return LOGIN_ACTION_NONE;
	if (State->auth_ping_pending)
		return LOGIN_ACTION_WAIT;
	if (State->has_nick && State->has_user)
		return LOGIN_ACTION_REGISTER;
	if (State->cap_pending)
		return LOGIN_ACTION_WAIT;
	return LOGIN_ACTION_NONE;
}

GLOBAL bool
Login_State_CanComplete(const CLIENT *Client)
{
	(void)Client;
	return true;
}

GLOBAL bool
Login_State_ShouldLoginNow(int client_type, bool got_nick, bool got_user,
			   bool cap_pending, bool auth_ping_pending)
{
	(void)client_type;
	if (auth_ping_pending)
		return false;
	if (cap_pending)
		return false;
	return got_nick && got_user;
}
