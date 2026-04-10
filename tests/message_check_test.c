#include "portab.h"
#include "message_check.h"

#include <stdio.h>

static int fail(const char *msg) { fprintf(stderr, "%s\n", msg); return 1; }

typedef struct _REQUEST
{
	char *prefix;
	char *command;
	char *argv[15];
	int argc;
} REQUEST;

int main(void)
{
	REQUEST r = {0};
	r.command = "PRIVMSG";
	r.argc = 0;
	if (Message_Check_Send(NULL, &r, 0, true) != MESSAGE_CHECK_NO_RECIPIENT)
		return fail("missing recipient");
	r.argc = 1;
	r.argv[0] = "nick";
	r.argv[1] = "";
	if (Message_Check_Send(NULL, &r, 0, true) != MESSAGE_CHECK_NO_TEXT)
		return fail("missing text");
	r.argc = 3;
	if (Message_Check_Send(NULL, &r, 0, true) != MESSAGE_CHECK_NEED_MORE_PARAMS)
		return fail("too many params");
	r.command = "JOIN";
	r.argc = 1;
	r.argv[0] = "";
	if (Message_Check_Join(NULL, &r) != MESSAGE_CHECK_NEED_MORE_PARAMS)
		return fail("empty join channel");
	return 0;
}
