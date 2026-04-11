#include "portab.h"

#include "nick_check.h"

GLOBAL NICK_CHECK_RESULT
Nick_Check_Context(int client_type, int argc)
{
	if (client_type == NICK_CLIENT_SERVER && argc == 1)
		return NICK_CHECK_USER_OR_CHANGE;
	if (client_type == NICK_CLIENT_SERVER || client_type == NICK_CLIENT_SERVICE) {
		if (argc == 2 || argc == 7)
			return NICK_CHECK_SERVER_INTRODUCE;
		return NICK_CHECK_ALREADY_REGISTERED;
	}

	if (client_type == NICK_CLIENT_UNKNOWN
	    || client_type == NICK_CLIENT_GOTPASS
	    || client_type == NICK_CLIENT_GOTNICK
#ifndef STRICT_RFC
	    || client_type == NICK_CLIENT_GOTUSER
#endif
	    || client_type == NICK_CLIENT_USER
	    || client_type == NICK_CLIENT_SERVICE)
		return NICK_CHECK_USER_OR_CHANGE;

	return NICK_CHECK_ALREADY_REGISTERED;
}
