#include "portab.h"
#include "login_check.h"
#include "join_check.h"
#include "join_error.h"
#include "login_flow.h"
#include "nick_check.h"
#include "user_check.h"
#include "message_check.h"

#include <stdio.h>
#include <string.h>

static int fail(const char *name) { fprintf(stderr, "%s\n", name); return 1; }

typedef struct _REQUEST {
    char *prefix;
    char *command;
    char *argv[15];
    int argc;
} REQUEST;

#define CHECK(expr, name) do { if (!(expr)) return fail(name); } while (0)

int main(void)
{
    /* 20 PASS/login checks */
    CHECK(Login_Check_Pass(LOGIN_CLIENT_UNKNOWN, 1) == LOGIN_CHECK_ALLOWED, "p01");
    CHECK(Login_Check_Pass(LOGIN_CLIENT_UNKNOWN, 2) == LOGIN_CHECK_NEEDMOREPARAMS, "p02");
    CHECK(Login_Check_Pass(LOGIN_CLIENT_UNKNOWN, 3) == LOGIN_CHECK_ALLOWED, "p03");
    CHECK(Login_Check_Pass(LOGIN_CLIENT_UNKNOWN, 4) == LOGIN_CHECK_ALLOWED, "p04");
    CHECK(Login_Check_Pass(LOGIN_CLIENT_USER, 1) == LOGIN_CHECK_ALREADYREGISTERED, "p05");
    CHECK(Login_Check_Pass(LOGIN_CLIENT_UNKNOWNSERVER, 2) == LOGIN_CHECK_NEEDMOREPARAMS, "p06");
    CHECK(Login_Check_Pass(LOGIN_CLIENT_UNKNOWNSERVER, 3) == LOGIN_CHECK_ALLOWED, "p07");
    CHECK(Login_Check_Pass(LOGIN_CLIENT_UNKNOWNSERVER, 4) == LOGIN_CHECK_ALLOWED, "p08");
    CHECK(Login_Check_Pass(LOGIN_CLIENT_UNKNOWNSERVER, 1) == LOGIN_CHECK_NEEDMOREPARAMS, "p09");
    CHECK(Login_Check_Pass(LOGIN_CLIENT_UNKNOWN, 0) == LOGIN_CHECK_NEEDMOREPARAMS, "p10");
    CHECK(Login_Check_Pass(LOGIN_CLIENT_GOTPASS, 1) == LOGIN_CHECK_ALREADYREGISTERED, "p11");
    CHECK(Login_Check_Pass(LOGIN_CLIENT_GOTPASS, 3) == LOGIN_CHECK_ALREADYREGISTERED, "p12");
    CHECK(Login_Check_Pass(LOGIN_CLIENT_GOTPASS, 4) == LOGIN_CHECK_ALREADYREGISTERED, "p13");
    CHECK(Login_Check_Pass(LOGIN_CLIENT_USER, 2) == LOGIN_CHECK_ALREADYREGISTERED, "p14");
    CHECK(Login_Check_Pass(LOGIN_CLIENT_SERVICE, 2) == LOGIN_CHECK_ALREADYREGISTERED, "p15");
    CHECK(Login_Check_Pass(LOGIN_CLIENT_SERVICE, 4) == LOGIN_CHECK_ALREADYREGISTERED, "p16");
    CHECK(Login_Check_Pass(LOGIN_CLIENT_UNKNOWN, 999) == LOGIN_CHECK_NEEDMOREPARAMS, "p17");
    CHECK(Login_Check_Pass(LOGIN_CLIENT_UNKNOWNSERVER, 999) == LOGIN_CHECK_NEEDMOREPARAMS, "p18");
    CHECK(Login_Check_Pass(LOGIN_CLIENT_USER, 999) == LOGIN_CHECK_ALREADYREGISTERED, "p19");
    CHECK(Login_Check_Pass(LOGIN_CLIENT_USER, 999) == LOGIN_CHECK_ALREADYREGISTERED, "p20");

    /* 16 JOIN access checks */
    CHECK(Join_Check_Access(false,false,false,false,true,false,false,false,false,false,false) == JOIN_CHECK_OK, "j01");
    CHECK(Join_Check_Access(true,false,false,false,true,false,false,false,false,false,false) == JOIN_CHECK_OK, "j02");
    CHECK(Join_Check_Access(false,true,false,false,true,false,false,false,false,false,false) == JOIN_CHECK_BANNED, "j03");
    CHECK(Join_Check_Access(false,true,true,false,true,false,false,false,false,false,false) == JOIN_CHECK_OK, "j04");
    CHECK(Join_Check_Access(false,true,false,true,true,false,false,false,false,false,false) == JOIN_CHECK_OK, "j05");
    CHECK(Join_Check_Access(false,false,false,false,false,false,false,false,false,false,false) == JOIN_CHECK_BADKEY, "j06");
    CHECK(Join_Check_Access(false,false,false,false,true,true,false,false,false,false,false) == JOIN_CHECK_FULL, "j07");
    CHECK(Join_Check_Access(false,false,false,false,true,false,true,false,false,false,false) == JOIN_CHECK_SECUREONLY, "j08");
    CHECK(Join_Check_Access(false,false,false,false,true,false,false,true,false,false,false) == JOIN_CHECK_OK, "j09");
    CHECK(Join_Check_Access(false,false,false,false,true,false,false,false,true,false,false) == JOIN_CHECK_OPONLY, "j10");
    CHECK(Join_Check_Access(true,false,false,false,true,false,false,false,true,false,false) == JOIN_CHECK_OK, "j11");
    CHECK(Join_Check_Access(false,false,false,false,true,false,false,false,false,true,false) == JOIN_CHECK_REGONLY, "j12");
    CHECK(Join_Check_Access(false,false,false,false,true,false,false,false,false,false,true) == JOIN_CHECK_OK, "j13");
    CHECK(Join_Check_Access(false,false,false,false,true,false,false,false,false,false,false) == JOIN_CHECK_OK, "j14");
    CHECK(Join_Check_Access(false,false,false,false,true,false,true,false,false,false,false) == JOIN_CHECK_SECUREONLY, "j15");
    CHECK(Join_Check_Access(false,false,false,false,true,true,true,false,false,false,false) == JOIN_CHECK_FULL, "j16");

    /* 8 join error mappings */
    CHECK(Join_Error_FromCheck(JOIN_CHECK_OK) == JOIN_ERROR_NONE, "e01");
    CHECK(Join_Error_FromCheck(JOIN_CHECK_BANNED) == JOIN_ERROR_BANNED, "e02");
    CHECK(Join_Error_FromCheck(JOIN_CHECK_INVITEONLY) == JOIN_ERROR_INVITEONLY, "e03");
    CHECK(Join_Error_FromCheck(JOIN_CHECK_BADKEY) == JOIN_ERROR_BADKEY, "e04");
    CHECK(Join_Error_FromCheck(JOIN_CHECK_FULL) == JOIN_ERROR_FULL, "e05");
    CHECK(Join_Error_FromCheck(JOIN_CHECK_SECUREONLY) == JOIN_ERROR_SECUREONLY, "e06");
    CHECK(Join_Error_FromCheck(JOIN_CHECK_OPONLY) == JOIN_ERROR_OPONLY, "e07");
    CHECK(Join_Error_FromCheck(JOIN_CHECK_REGONLY) == JOIN_ERROR_REGONLY, "e08");

    /* 10 login flow cases */
    CHECK(Login_Flow_OnNick(LOGIN_CLIENT_UNKNOWN, true, false, false, true) == LOGIN_FLOW_LOGIN_NOW, "f01");
    CHECK(Login_Flow_OnNick(LOGIN_CLIENT_UNKNOWN, false, false, false, true) == LOGIN_FLOW_SET_GOTNICK, "f02");
    CHECK(Login_Flow_OnNick(LOGIN_CLIENT_UNKNOWN, false, true, false, true) == LOGIN_FLOW_SET_GOTNICK, "f03");
    CHECK(Login_Flow_OnNick(LOGIN_CLIENT_GOTPASS, true, false, false, true) == LOGIN_FLOW_LOGIN_NOW, "f04");
    CHECK(Login_Flow_OnNick(LOGIN_CLIENT_GOTPASS, false, true, false, true) == LOGIN_FLOW_SET_GOTNICK, "f05");
    CHECK(Login_Flow_OnUser(LOGIN_CLIENT_GOTNICK, true, false, false, true) == LOGIN_FLOW_LOGIN_NOW, "f06");
    CHECK(Login_Flow_OnUser(LOGIN_CLIENT_GOTNICK, false, false, false, true) == LOGIN_FLOW_SET_GOTUSER, "f07");
    CHECK(Login_Flow_OnUser(LOGIN_CLIENT_GOTPASS, true, true, false, true) == LOGIN_FLOW_SET_GOTUSER, "f08");
    CHECK(Login_Flow_OnUser(LOGIN_CLIENT_USER, true, false, false, true) == LOGIN_FLOW_KEEP_STATE, "f09");
#ifdef STRICT_RFC
    CHECK(Login_Flow_OnUser(LOGIN_CLIENT_UNKNOWN, true, false, false, true) == LOGIN_FLOW_KEEP_STATE, "f10");
#else
    CHECK(Login_Flow_OnUser(LOGIN_CLIENT_UNKNOWN, true, false, false, true) == LOGIN_FLOW_LOGIN_NOW, "f10");
#endif

    /* 8 nick state cases */
    CHECK(Nick_Check_Context(NICK_CLIENT_UNKNOWN, 1) == NICK_CHECK_USER_OR_CHANGE, "n01");
    CHECK(Nick_Check_Context(NICK_CLIENT_GOTPASS, 1) == NICK_CHECK_USER_OR_CHANGE, "n02");
    CHECK(Nick_Check_Context(NICK_CLIENT_GOTNICK, 1) == NICK_CHECK_USER_OR_CHANGE, "n03");
#ifndef STRICT_RFC
    CHECK(Nick_Check_Context(NICK_CLIENT_GOTUSER, 1) == NICK_CHECK_USER_OR_CHANGE, "n04");
#endif
    CHECK(Nick_Check_Context(NICK_CLIENT_USER, 1) == NICK_CHECK_USER_OR_CHANGE, "n05");
    CHECK(Nick_Check_Context(NICK_CLIENT_SERVER, 2) == NICK_CHECK_SERVER_INTRODUCE, "n06");
    CHECK(Nick_Check_Context(NICK_CLIENT_SERVER, 7) == NICK_CHECK_SERVER_INTRODUCE, "n07");
    CHECK(Nick_Check_Context(NICK_CLIENT_SERVICE, 2) == NICK_CHECK_SERVER_INTRODUCE, "n08");

    /* 8 user state cases */
    CHECK(User_Check_Context(USER_CLIENT_GOTNICK) == USER_CHECK_NEW_CONNECTION, "u01");
    CHECK(User_Check_Context(USER_CLIENT_GOTPASS) == USER_CHECK_NEW_CONNECTION, "u02");
#ifdef STRICT_RFC
    CHECK(User_Check_Context(USER_CLIENT_UNKNOWN) == USER_CHECK_NOT_REGISTERED, "u03");
#else
    CHECK(User_Check_Context(USER_CLIENT_UNKNOWN) == USER_CHECK_NEW_CONNECTION, "u03");
#endif
    CHECK(User_Check_Context(USER_CLIENT_SERVER) == USER_CHECK_SERVER_UPDATE, "u04");
    CHECK(User_Check_Context(USER_CLIENT_SERVICE) == USER_CHECK_SERVER_UPDATE, "u05");
    CHECK(User_Check_Context(USER_CLIENT_USER) == USER_CHECK_ALREADY_REGISTERED, "u06");
    CHECK(User_Check_Context(USER_CLIENT_GOTNICK) == USER_CHECK_NEW_CONNECTION, "u07");
#ifdef STRICT_RFC
    CHECK(User_Check_Context(USER_CLIENT_UNKNOWN) == USER_CHECK_NOT_REGISTERED, "u08");
#else
    CHECK(User_Check_Context(USER_CLIENT_UNKNOWN) == USER_CHECK_NEW_CONNECTION, "u08");
#endif

    /* 30 message checks */
    REQUEST r = {0};
    r.command = "PRIVMSG";
    CHECK(Message_Check_Send(NULL, &r, 0, true) == MESSAGE_CHECK_NO_RECIPIENT, "m01");
    CHECK(Message_Check_Send(NULL, &r, 0, false) == MESSAGE_CHECK_OK, "m02");
    r.argc = 1; r.argv[0] = "nick";
    CHECK(Message_Check_Send(NULL, &r, 0, true) == MESSAGE_CHECK_NO_TEXT, "m03");
    CHECK(Message_Check_Send(NULL, &r, 0, false) == MESSAGE_CHECK_OK, "m04");
    r.argv[1] = "";
    CHECK(Message_Check_Send(NULL, &r, 0, true) == MESSAGE_CHECK_NO_TEXT, "m05");
    r.argv[1] = "hi";
    CHECK(Message_Check_Send(NULL, &r, 0, true) == MESSAGE_CHECK_NO_TEXT, "m06");
    r.argc = 2;
    CHECK(Message_Check_Send(NULL, &r, 0, true) == MESSAGE_CHECK_OK, "m07");
    r.argc = 3;
    CHECK(Message_Check_Send(NULL, &r, 0, true) == MESSAGE_CHECK_NEED_MORE_PARAMS, "m08");
    CHECK(Message_Check_Send(NULL, &r, 0, false) == MESSAGE_CHECK_OK, "m09");
    r.command = "NOTICE";
    CHECK(Message_Check_Send(NULL, &r, 0, true) == MESSAGE_CHECK_NEED_MORE_PARAMS, "m10");
    CHECK(Message_Check_Send(NULL, &r, 0, false) == MESSAGE_CHECK_OK, "m11");
    r.argc = 0;
    CHECK(Message_Check_Join(NULL, &r) == MESSAGE_CHECK_NEED_MORE_PARAMS, "m12");
    r.argc = 1; r.argv[0] = NULL;
    CHECK(Message_Check_Join(NULL, &r) == MESSAGE_CHECK_NEED_MORE_PARAMS, "m13");
    r.argv[0] = "";
    CHECK(Message_Check_Join(NULL, &r) == MESSAGE_CHECK_NEED_MORE_PARAMS, "m14");
    r.argv[0] = "#chan";
    CHECK(Message_Check_Join(NULL, &r) == MESSAGE_CHECK_OK, "m15");
    r.argv[0] = "&local";
    CHECK(Message_Check_Join(NULL, &r) == MESSAGE_CHECK_OK, "m16");
    r.argv[0] = "0";
    CHECK(Message_Check_Join(NULL, &r) == MESSAGE_CHECK_OK, "m17");
    r.argc = 2;
    CHECK(Message_Check_Join(NULL, &r) == MESSAGE_CHECK_OK, "m18");
    r.argc = 1; r.command = "PRIVMSG"; r.argv[0] = "nick"; r.argv[1] = NULL;
    CHECK(Message_Check_Send(NULL, &r, 0, true) == MESSAGE_CHECK_NO_TEXT, "m19");
    r.argv[1] = "text";
    CHECK(Message_Check_Send(NULL, &r, 0, true) == MESSAGE_CHECK_NO_TEXT, "m20");
    r.argc = 0; r.command = "NOTICE";
    CHECK(Message_Check_Send(NULL, &r, 0, false) == MESSAGE_CHECK_OK, "m21");
    r.argc = 1;
    CHECK(Message_Check_Send(NULL, &r, 0, false) == MESSAGE_CHECK_OK, "m22");
    r.argc = 2;
    CHECK(Message_Check_Send(NULL, &r, 0, false) == MESSAGE_CHECK_OK, "m23");
    r.argc = 3;
    CHECK(Message_Check_Send(NULL, &r, 0, false) == MESSAGE_CHECK_OK, "m24");
    r.argv[1] = "";
    CHECK(Message_Check_Send(NULL, &r, 0, false) == MESSAGE_CHECK_OK, "m25");
    r.argv[1] = "text";
    CHECK(Message_Check_Send(NULL, &r, 0, false) == MESSAGE_CHECK_OK, "m26");
    r.command = "JOIN"; r.argc = 0;
    CHECK(Message_Check_Join(NULL, &r) == MESSAGE_CHECK_NEED_MORE_PARAMS, "m27");
    r.argc = 1; r.argv[0] = NULL;
    CHECK(Message_Check_Join(NULL, &r) == MESSAGE_CHECK_NEED_MORE_PARAMS, "m28");
    r.argv[0] = "#x";
    CHECK(Message_Check_Join(NULL, &r) == MESSAGE_CHECK_OK, "m29");
    r.argv[0] = "#y";
    CHECK(Message_Check_Join(NULL, &r) == MESSAGE_CHECK_OK, "m30");

    return 0;
}
