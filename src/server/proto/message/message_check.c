#include "portab.h"

#include "message_check.h"

struct _REQUEST
{
	char *prefix;
	char *command;
	char *argv[15];
	int argc;
};

GLOBAL MESSAGE_CHECK_RESULT
Message_Check_Send(const void *Client, const struct _REQUEST *Req, int ForceType,
		   bool SendErrors)
{
	(void)Client;
	(void)ForceType;
	if (Req->argc == 0)
		return SendErrors ? MESSAGE_CHECK_NO_RECIPIENT : MESSAGE_CHECK_OK;
	if (Req->argc == 1)
		return SendErrors ? MESSAGE_CHECK_NO_TEXT : MESSAGE_CHECK_OK;
	if (Req->argc > 2)
		return SendErrors ? MESSAGE_CHECK_NEED_MORE_PARAMS : MESSAGE_CHECK_OK;
	if (Req->argv[1] == NULL || Req->argv[1][0] == '\0')
		return SendErrors ? MESSAGE_CHECK_NO_TEXT : MESSAGE_CHECK_OK;
	return MESSAGE_CHECK_OK;
}

GLOBAL MESSAGE_CHECK_RESULT
Message_Check_Join(const void *Client, const struct _REQUEST *Req)
{
	(void)Client;
	if (Req->argc == 0 || Req->argv[0] == NULL || Req->argv[0][0] == '\0')
		return MESSAGE_CHECK_NEED_MORE_PARAMS;
	return MESSAGE_CHECK_OK;
}
