#include "portab.h"

#include "login_check.h"

GLOBAL LOGIN_CHECK_RESULT
Login_Check_Pass(int client_type, int argc)
{
	if (client_type != LOGIN_CLIENT_UNKNOWN && client_type != LOGIN_CLIENT_UNKNOWNSERVER)
		return LOGIN_CHECK_ALREADYREGISTERED;

	if (client_type == LOGIN_CLIENT_UNKNOWN && argc == 1)
		return LOGIN_CHECK_ALLOWED;
	if ((client_type == LOGIN_CLIENT_UNKNOWN || client_type == LOGIN_CLIENT_UNKNOWNSERVER)
	    && (argc == 3 || argc == 4))
		return LOGIN_CHECK_ALLOWED;

	return LOGIN_CHECK_NEEDMOREPARAMS;
}
