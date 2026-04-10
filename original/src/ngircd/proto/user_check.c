#include "portab.h"

#include "user_check.h"

GLOBAL USER_CHECK_RESULT
User_Check_Context(int client_type)
{
	if (client_type == USER_CLIENT_GOTNICK
#ifndef STRICT_RFC
	    || client_type == USER_CLIENT_UNKNOWN
#endif
	    || client_type == USER_CLIENT_GOTPASS)
		return USER_CHECK_NEW_CONNECTION;

	if (client_type == USER_CLIENT_SERVER || client_type == USER_CLIENT_SERVICE)
		return USER_CHECK_SERVER_UPDATE;

	if (client_type == USER_CLIENT_USER)
		return USER_CHECK_ALREADY_REGISTERED;

	return USER_CHECK_NOT_REGISTERED;
}
