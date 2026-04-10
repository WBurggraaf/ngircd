/*
 * Comprehensive coverage test for all extracted modules.
 * Targets every branch/condition to achieve 100% code coverage.
 */

#include "portab.h"
#include "login_check.h"
#include "join_check.h"
#include "join_error.h"
#include "login_flow.h"
#include "login_state.h"
#include "nick_check.h"
#include "user_check.h"
#include "message_check.h"
#include "module_iface.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;
static int tests = 0;

#define T(expr, name) do { \
	tests++; \
	if (!(expr)) { \
		fprintf(stderr, "FAIL: %s (line %d)\n", name, __LINE__); \
		failures++; \
	} \
} while (0)

typedef struct _REQUEST {
	char *prefix;
	char *command;
	char *argv[15];
	int argc;
} REQUEST;

/* =========== join_check.c =========== */
static void test_join_check(void)
{
	/* Basic OK: no restrictions */
	T(Join_Check_Access(false,false,false,false,true,false,false,false,false,false,false) == JOIN_CHECK_OK, "jc: basic ok");

	/* Operator bypasses everything */
	T(Join_Check_Access(true,false,false,false,true,false,false,false,false,false,false) == JOIN_CHECK_OK, "jc: operator ok");
	T(Join_Check_Access(true,true,false,false,false,true,true,false,true,true,false) == JOIN_CHECK_OK, "jc: operator bypasses all");

	/* Banned: banned && !invited && !exception */
	T(Join_Check_Access(false,true,false,false,true,false,false,false,false,false,false) == JOIN_CHECK_BANNED, "jc: banned");
	/* Banned but invited → OK */
	T(Join_Check_Access(false,true,true,false,true,false,false,false,false,false,false) == JOIN_CHECK_OK, "jc: banned+invited");
	/* Banned but exception → OK */
	T(Join_Check_Access(false,true,false,true,true,false,false,false,false,false,false) == JOIN_CHECK_OK, "jc: banned+exception");
	/* Banned + invited + exception → OK */
	T(Join_Check_Access(false,true,true,true,true,false,false,false,false,false,false) == JOIN_CHECK_OK, "jc: banned+both");

	/* Bad key */
	T(Join_Check_Access(false,false,false,false,false,false,false,false,false,false,false) == JOIN_CHECK_BADKEY, "jc: badkey");

	/* Full */
	T(Join_Check_Access(false,false,false,false,true,true,false,false,false,false,false) == JOIN_CHECK_FULL, "jc: full");

	/* Secure only: no SSL */
	T(Join_Check_Access(false,false,false,false,true,false,true,false,false,false,false) == JOIN_CHECK_SECUREONLY, "jc: secure no ssl");
	/* Secure only: has SSL → OK */
	T(Join_Check_Access(false,false,false,false,true,false,true,true,false,false,false) == JOIN_CHECK_OK, "jc: secure has ssl");

	/* Oper only (non-operator) */
	T(Join_Check_Access(false,false,false,false,true,false,false,false,true,false,false) == JOIN_CHECK_OPONLY, "jc: oper only");

	/* Reg only: not registered */
	T(Join_Check_Access(false,false,false,false,true,false,false,false,false,true,false) == JOIN_CHECK_REGONLY, "jc: reg only unreg");
	/* Reg only: registered → OK */
	T(Join_Check_Access(false,false,false,false,true,false,false,false,false,true,true) == JOIN_CHECK_OK, "jc: reg only registered");

	/* Priority: full before secure */
	T(Join_Check_Access(false,false,false,false,true,true,true,false,false,false,false) == JOIN_CHECK_FULL, "jc: full before secure");
	/* Priority: badkey before full */
	T(Join_Check_Access(false,false,false,false,false,true,false,false,false,false,false) == JOIN_CHECK_BADKEY, "jc: badkey before full");
	/* Priority: banned before badkey */
	T(Join_Check_Access(false,true,false,false,false,false,false,false,false,false,false) == JOIN_CHECK_BANNED, "jc: banned before badkey");
}

/* =========== join_error.c =========== */
static void test_join_error(void)
{
	T(Join_Error_FromCheck(JOIN_CHECK_OK) == JOIN_ERROR_NONE, "je: ok→none");
	T(Join_Error_FromCheck(JOIN_CHECK_BANNED) == JOIN_ERROR_BANNED, "je: banned");
	T(Join_Error_FromCheck(JOIN_CHECK_INVITEONLY) == JOIN_ERROR_INVITEONLY, "je: inviteonly");
	T(Join_Error_FromCheck(JOIN_CHECK_BADKEY) == JOIN_ERROR_BADKEY, "je: badkey");
	T(Join_Error_FromCheck(JOIN_CHECK_FULL) == JOIN_ERROR_FULL, "je: full");
	T(Join_Error_FromCheck(JOIN_CHECK_SECUREONLY) == JOIN_ERROR_SECUREONLY, "je: secure");
	T(Join_Error_FromCheck(JOIN_CHECK_OPONLY) == JOIN_ERROR_OPONLY, "je: oponly");
	T(Join_Error_FromCheck(JOIN_CHECK_REGONLY) == JOIN_ERROR_REGONLY, "je: regonly");
	/* Default/unknown value → NONE */
	T(Join_Error_FromCheck((JOIN_CHECK_RESULT)999) == JOIN_ERROR_NONE, "je: default→none");
}

/* =========== login_check.c =========== */
static void test_login_check(void)
{
	/* UNKNOWN client */
	T(Login_Check_Pass(LOGIN_CLIENT_UNKNOWN, 0) == LOGIN_CHECK_NEEDMOREPARAMS, "lc: unknown 0");
	T(Login_Check_Pass(LOGIN_CLIENT_UNKNOWN, 1) == LOGIN_CHECK_ALLOWED, "lc: unknown 1 rfc1459");
	T(Login_Check_Pass(LOGIN_CLIENT_UNKNOWN, 2) == LOGIN_CHECK_NEEDMOREPARAMS, "lc: unknown 2");
	T(Login_Check_Pass(LOGIN_CLIENT_UNKNOWN, 3) == LOGIN_CHECK_ALLOWED, "lc: unknown 3 rfc2813");
	T(Login_Check_Pass(LOGIN_CLIENT_UNKNOWN, 4) == LOGIN_CHECK_ALLOWED, "lc: unknown 4 rfc2813");
	T(Login_Check_Pass(LOGIN_CLIENT_UNKNOWN, 5) == LOGIN_CHECK_NEEDMOREPARAMS, "lc: unknown 5");

	/* UNKNOWNSERVER client */
	T(Login_Check_Pass(LOGIN_CLIENT_UNKNOWNSERVER, 0) == LOGIN_CHECK_NEEDMOREPARAMS, "lc: server 0");
	T(Login_Check_Pass(LOGIN_CLIENT_UNKNOWNSERVER, 1) == LOGIN_CHECK_NEEDMOREPARAMS, "lc: server 1");
	T(Login_Check_Pass(LOGIN_CLIENT_UNKNOWNSERVER, 2) == LOGIN_CHECK_NEEDMOREPARAMS, "lc: server 2");
	T(Login_Check_Pass(LOGIN_CLIENT_UNKNOWNSERVER, 3) == LOGIN_CHECK_ALLOWED, "lc: server 3");
	T(Login_Check_Pass(LOGIN_CLIENT_UNKNOWNSERVER, 4) == LOGIN_CHECK_ALLOWED, "lc: server 4");
	T(Login_Check_Pass(LOGIN_CLIENT_UNKNOWNSERVER, 5) == LOGIN_CHECK_NEEDMOREPARAMS, "lc: server 5");

	/* Already registered client types */
	T(Login_Check_Pass(LOGIN_CLIENT_USER, 1) == LOGIN_CHECK_ALREADYREGISTERED, "lc: user reg");
	T(Login_Check_Pass(LOGIN_CLIENT_USER, 3) == LOGIN_CHECK_ALREADYREGISTERED, "lc: user reg 3");
	T(Login_Check_Pass(0x0002, 1) == LOGIN_CHECK_ALREADYREGISTERED, "lc: gotpass reg");
	T(Login_Check_Pass(0x0004, 1) == LOGIN_CHECK_ALREADYREGISTERED, "lc: gotnick reg");
	T(Login_Check_Pass(0x0040, 1) == LOGIN_CHECK_ALREADYREGISTERED, "lc: service reg");
}

/* =========== login_flow.c =========== */
static void test_login_flow(void)
{
	/* OnNick: UNKNOWN + user received + registration allowed → LOGIN_NOW */
	T(Login_Flow_OnNick(LOGIN_CLIENT_UNKNOWN, true, false, false, true) == LOGIN_FLOW_LOGIN_NOW, "lf: nick unknown login");
	/* OnNick: UNKNOWN + no user → SET_GOTNICK */
	T(Login_Flow_OnNick(LOGIN_CLIENT_UNKNOWN, false, false, false, true) == LOGIN_FLOW_SET_GOTNICK, "lf: nick unknown set");
	/* OnNick: UNKNOWN + cap_pending → SET_GOTNICK (blocked by pending) */
	T(Login_Flow_OnNick(LOGIN_CLIENT_UNKNOWN, true, true, false, true) == LOGIN_FLOW_SET_GOTNICK, "lf: nick cap pending");
	/* OnNick: UNKNOWN + auth_ping_pending → SET_GOTNICK */
	T(Login_Flow_OnNick(LOGIN_CLIENT_UNKNOWN, true, false, true, true) == LOGIN_FLOW_SET_GOTNICK, "lf: nick auth pending");
	/* OnNick: UNKNOWN + !registration_allowed → SET_GOTNICK */
	T(Login_Flow_OnNick(LOGIN_CLIENT_UNKNOWN, true, false, false, false) == LOGIN_FLOW_SET_GOTNICK, "lf: nick no reg");

	/* OnNick: GOTPASS client */
	T(Login_Flow_OnNick(LOGIN_CLIENT_GOTPASS, true, false, false, true) == LOGIN_FLOW_LOGIN_NOW, "lf: nick gotpass login");
	T(Login_Flow_OnNick(LOGIN_CLIENT_GOTPASS, false, false, false, true) == LOGIN_FLOW_SET_GOTNICK, "lf: nick gotpass set");

	/* OnNick: GOTNICK client */
	T(Login_Flow_OnNick(LOGIN_CLIENT_GOTNICK, true, false, false, true) == LOGIN_FLOW_LOGIN_NOW, "lf: nick gotnick login");
	T(Login_Flow_OnNick(LOGIN_CLIENT_GOTNICK, false, false, false, true) == LOGIN_FLOW_SET_GOTNICK, "lf: nick gotnick set");

	/* OnNick: USER client */
	T(Login_Flow_OnNick(LOGIN_CLIENT_USER, true, false, false, true) == LOGIN_FLOW_LOGIN_NOW, "lf: nick user login");
	T(Login_Flow_OnNick(LOGIN_CLIENT_USER, false, false, false, true) == LOGIN_FLOW_SET_GOTNICK, "lf: nick user set");

	/* OnNick: SERVICE client */
	T(Login_Flow_OnNick(LOGIN_CLIENT_SERVICE, true, false, false, true) == LOGIN_FLOW_LOGIN_NOW, "lf: nick service login");
	T(Login_Flow_OnNick(LOGIN_CLIENT_SERVICE, false, false, false, true) == LOGIN_FLOW_SET_GOTNICK, "lf: nick service set");

	/* OnNick: default (unrecognized type) → KEEP_STATE */
	T(Login_Flow_OnNick(0x0100, true, false, false, true) == LOGIN_FLOW_KEEP_STATE, "lf: nick default keep");

#ifdef STRICT_RFC
	/* With STRICT_RFC: GOTUSER is NOT in OnNick switch → KEEP_STATE */
	T(Login_Flow_OnNick(LOGIN_CLIENT_GOTUSER, true, false, false, true) == LOGIN_FLOW_KEEP_STATE, "lf: nick gotuser strict");
#else
	/* Without STRICT_RFC: GOTUSER IS in OnNick switch */
	T(Login_Flow_OnNick(LOGIN_CLIENT_GOTUSER, true, false, false, true) == LOGIN_FLOW_LOGIN_NOW, "lf: nick gotuser nonstrict");
#endif

	/* OnUser: GOTNICK + nick received → LOGIN_NOW */
	T(Login_Flow_OnUser(LOGIN_CLIENT_GOTNICK, true, false, false, true) == LOGIN_FLOW_LOGIN_NOW, "lf: user gotnick login");
	T(Login_Flow_OnUser(LOGIN_CLIENT_GOTNICK, false, false, false, true) == LOGIN_FLOW_SET_GOTUSER, "lf: user gotnick set");

	/* OnUser: GOTPASS */
	T(Login_Flow_OnUser(LOGIN_CLIENT_GOTPASS, true, false, false, true) == LOGIN_FLOW_LOGIN_NOW, "lf: user gotpass login");
	T(Login_Flow_OnUser(LOGIN_CLIENT_GOTPASS, false, false, false, true) == LOGIN_FLOW_SET_GOTUSER, "lf: user gotpass set");

	/* OnUser: pending flags */
	T(Login_Flow_OnUser(LOGIN_CLIENT_GOTNICK, true, true, false, true) == LOGIN_FLOW_SET_GOTUSER, "lf: user cap pending");
	T(Login_Flow_OnUser(LOGIN_CLIENT_GOTNICK, true, false, true, true) == LOGIN_FLOW_SET_GOTUSER, "lf: user auth pending");
	T(Login_Flow_OnUser(LOGIN_CLIENT_GOTNICK, true, false, false, false) == LOGIN_FLOW_SET_GOTUSER, "lf: user no reg");

	/* OnUser: default → KEEP_STATE */
	T(Login_Flow_OnUser(LOGIN_CLIENT_USER, false, false, false, true) == LOGIN_FLOW_KEEP_STATE, "lf: user user keep");
	T(Login_Flow_OnUser(LOGIN_CLIENT_SERVICE, false, false, false, true) == LOGIN_FLOW_KEEP_STATE, "lf: user service keep");
	T(Login_Flow_OnUser(0x0100, true, false, false, true) == LOGIN_FLOW_KEEP_STATE, "lf: user default keep");

#ifdef STRICT_RFC
	/* With STRICT_RFC: UNKNOWN is NOT in OnUser switch → KEEP_STATE */
	T(Login_Flow_OnUser(LOGIN_CLIENT_UNKNOWN, true, false, false, true) == LOGIN_FLOW_KEEP_STATE, "lf: user unknown strict");
#else
	T(Login_Flow_OnUser(LOGIN_CLIENT_UNKNOWN, true, false, false, true) == LOGIN_FLOW_LOGIN_NOW, "lf: user unknown nonstrict");
#endif
}

/* =========== login_state.c =========== */
static void test_login_state(void)
{
	LOGIN_STATE s = {0};

	/* NULL → REJECT */
	T(Login_State_Resolve(NULL) == LOGIN_ACTION_REJECT, "ls: null reject");

	/* Empty state → NONE */
	T(Login_State_Resolve(&s) == LOGIN_ACTION_NONE, "ls: empty none");

	/* is_registered → NONE (takes priority) */
	s.is_registered = true;
	s.has_nick = true;
	s.has_user = true;
	T(Login_State_Resolve(&s) == LOGIN_ACTION_NONE, "ls: registered none");

	/* auth_ping_pending → WAIT */
	memset(&s, 0, sizeof(s));
	s.auth_ping_pending = true;
	s.has_nick = true;
	s.has_user = true;
	T(Login_State_Resolve(&s) == LOGIN_ACTION_WAIT, "ls: auth ping wait");

	/* has_nick + has_user → REGISTER */
	memset(&s, 0, sizeof(s));
	s.has_nick = true;
	s.has_user = true;
	T(Login_State_Resolve(&s) == LOGIN_ACTION_REGISTER, "ls: nick+user register");

	/* cap_pending → WAIT */
	memset(&s, 0, sizeof(s));
	s.cap_pending = true;
	T(Login_State_Resolve(&s) == LOGIN_ACTION_WAIT, "ls: cap pending wait");

	/* nick+user must register even if CAP negotiation is pending */
	memset(&s, 0, sizeof(s));
	s.has_nick = true;
	s.has_user = true;
	s.cap_pending = true;
	T(Login_State_Resolve(&s) == LOGIN_ACTION_REGISTER, "ls: cap pending does not block register");

	/* auth ping still blocks registration */
	memset(&s, 0, sizeof(s));
	s.has_nick = true;
	s.has_user = true;
	s.auth_ping_pending = true;
	T(Login_State_Resolve(&s) == LOGIN_ACTION_WAIT, "ls: auth ping priority");

	/* has_nick only → NONE */
	memset(&s, 0, sizeof(s));
	s.has_nick = true;
	T(Login_State_Resolve(&s) == LOGIN_ACTION_NONE, "ls: nick only none");

	/* has_user only → NONE */
	memset(&s, 0, sizeof(s));
	s.has_user = true;
	T(Login_State_Resolve(&s) == LOGIN_ACTION_NONE, "ls: user only none");

	/* cap_pending but nick only (not complete) → WAIT */
	memset(&s, 0, sizeof(s));
	s.has_nick = true;
	s.cap_pending = true;
	T(Login_State_Resolve(&s) == LOGIN_ACTION_WAIT, "ls: cap pending nick only");

	/* Login_State_CanComplete always returns true */
	T(Login_State_CanComplete(NULL) == true, "ls: can complete null");

	/* ShouldLoginNow */
	T(Login_State_ShouldLoginNow(0, true, true, false, false) == true, "ls: should login yes");
	T(Login_State_ShouldLoginNow(0, true, false, false, false) == false, "ls: no user");
	T(Login_State_ShouldLoginNow(0, false, true, false, false) == false, "ls: no nick");
	T(Login_State_ShouldLoginNow(0, false, false, false, false) == false, "ls: neither");
	T(Login_State_ShouldLoginNow(0, true, true, true, false) == false, "ls: cap pending blocks");
	T(Login_State_ShouldLoginNow(0, true, true, false, true) == false, "ls: auth pending blocks");
	T(Login_State_ShouldLoginNow(0, true, true, true, true) == false, "ls: both pending");
}

/* =========== nick_check.c =========== */
static void test_nick_check(void)
{
	/* Standard user types → USER_OR_CHANGE */
	T(Nick_Check_Context(NICK_CLIENT_UNKNOWN, 1) == NICK_CHECK_USER_OR_CHANGE, "nc: unknown");
	T(Nick_Check_Context(NICK_CLIENT_GOTPASS, 1) == NICK_CHECK_USER_OR_CHANGE, "nc: gotpass");
	T(Nick_Check_Context(NICK_CLIENT_GOTNICK, 1) == NICK_CHECK_USER_OR_CHANGE, "nc: gotnick");
	T(Nick_Check_Context(NICK_CLIENT_USER, 1) == NICK_CHECK_USER_OR_CHANGE, "nc: user");

#ifndef STRICT_RFC
	T(Nick_Check_Context(NICK_CLIENT_GOTUSER, 1) == NICK_CHECK_USER_OR_CHANGE, "nc: gotuser nonstrict");
#endif

	/* SERVICE argc=1: caught by second if (SERVER||SERVICE), argc!=2&&!=7 → ALREADY_REGISTERED */
	T(Nick_Check_Context(NICK_CLIENT_SERVICE, 1) == NICK_CHECK_ALREADY_REGISTERED, "nc: service argc1 reg");

	/* SERVER argc=1 → USER_OR_CHANGE (first if) */
	T(Nick_Check_Context(NICK_CLIENT_SERVER, 1) == NICK_CHECK_USER_OR_CHANGE, "nc: server argc1");

	/* SERVER argc=2 → SERVER_INTRODUCE */
	T(Nick_Check_Context(NICK_CLIENT_SERVER, 2) == NICK_CHECK_SERVER_INTRODUCE, "nc: server argc2");
	/* SERVER argc=7 → SERVER_INTRODUCE */
	T(Nick_Check_Context(NICK_CLIENT_SERVER, 7) == NICK_CHECK_SERVER_INTRODUCE, "nc: server argc7");
	/* SERVER argc=3 (not 1,2,7) → ALREADY_REGISTERED */
	T(Nick_Check_Context(NICK_CLIENT_SERVER, 3) == NICK_CHECK_ALREADY_REGISTERED, "nc: server argc3 reg");
	T(Nick_Check_Context(NICK_CLIENT_SERVER, 0) == NICK_CHECK_ALREADY_REGISTERED, "nc: server argc0 reg");
	T(Nick_Check_Context(NICK_CLIENT_SERVER, 5) == NICK_CHECK_ALREADY_REGISTERED, "nc: server argc5 reg");

	/* SERVICE argc=2 → SERVER_INTRODUCE */
	T(Nick_Check_Context(NICK_CLIENT_SERVICE, 2) == NICK_CHECK_SERVER_INTRODUCE, "nc: service argc2");
	/* SERVICE argc=7 → SERVER_INTRODUCE */
	T(Nick_Check_Context(NICK_CLIENT_SERVICE, 7) == NICK_CHECK_SERVER_INTRODUCE, "nc: service argc7");
	/* SERVICE argc=3 (not 2,7) → ALREADY_REGISTERED */
	T(Nick_Check_Context(NICK_CLIENT_SERVICE, 3) == NICK_CHECK_ALREADY_REGISTERED, "nc: service argc3 reg");

	/* Default: unknown client type → ALREADY_REGISTERED */
#ifdef STRICT_RFC
	T(Nick_Check_Context(NICK_CLIENT_GOTUSER, 1) == NICK_CHECK_ALREADY_REGISTERED, "nc: gotuser strict reg");
#endif
	T(Nick_Check_Context(0x0080, 1) == NICK_CHECK_ALREADY_REGISTERED, "nc: unknown type reg");
	T(Nick_Check_Context(0x0100, 1) == NICK_CHECK_ALREADY_REGISTERED, "nc: bad type reg");
}

/* =========== user_check.c =========== */
static void test_user_check(void)
{
	T(User_Check_Context(USER_CLIENT_GOTNICK) == USER_CHECK_NEW_CONNECTION, "uc: gotnick new");
	T(User_Check_Context(USER_CLIENT_GOTPASS) == USER_CHECK_NEW_CONNECTION, "uc: gotpass new");

#ifndef STRICT_RFC
	T(User_Check_Context(USER_CLIENT_UNKNOWN) == USER_CHECK_NEW_CONNECTION, "uc: unknown new nonstrict");
#else
	T(User_Check_Context(USER_CLIENT_UNKNOWN) == USER_CHECK_NOT_REGISTERED, "uc: unknown not reg strict");
#endif

	T(User_Check_Context(USER_CLIENT_SERVER) == USER_CHECK_SERVER_UPDATE, "uc: server update");
	T(User_Check_Context(USER_CLIENT_SERVICE) == USER_CHECK_SERVER_UPDATE, "uc: service update");
	T(User_Check_Context(USER_CLIENT_USER) == USER_CHECK_ALREADY_REGISTERED, "uc: user registered");

	/* GOTUSER → default → NOT_REGISTERED */
	T(User_Check_Context(USER_CLIENT_GOTUSER) == USER_CHECK_NOT_REGISTERED, "uc: gotuser not reg");
	/* Unknown type → NOT_REGISTERED */
	T(User_Check_Context(0x0080) == USER_CHECK_NOT_REGISTERED, "uc: unknown type not reg");
	T(User_Check_Context(0x0100) == USER_CHECK_NOT_REGISTERED, "uc: bad type not reg");
}

/* =========== message_check.c =========== */
static void test_message_check(void)
{
	REQUEST r;

	/* Send: argc=0, SendErrors=true → NO_RECIPIENT */
	memset(&r, 0, sizeof(r));
	r.command = "PRIVMSG";
	T(Message_Check_Send(NULL, &r, 0, true) == MESSAGE_CHECK_NO_RECIPIENT, "mc: no recipient err");
	/* Send: argc=0, SendErrors=false → OK */
	T(Message_Check_Send(NULL, &r, 0, false) == MESSAGE_CHECK_OK, "mc: no recipient silent");

	/* Send: argc=1, SendErrors=true → NO_TEXT */
	memset(&r, 0, sizeof(r));
	r.command = "PRIVMSG";
	r.argc = 1;
	r.argv[0] = "nick";
	T(Message_Check_Send(NULL, &r, 0, true) == MESSAGE_CHECK_NO_TEXT, "mc: no text err");
	T(Message_Check_Send(NULL, &r, 0, false) == MESSAGE_CHECK_OK, "mc: no text silent");

	/* Send: argc=2, argv[1]=NULL → NO_TEXT */
	memset(&r, 0, sizeof(r));
	r.command = "PRIVMSG";
	r.argc = 2;
	r.argv[0] = "nick";
	r.argv[1] = NULL;
	T(Message_Check_Send(NULL, &r, 0, true) == MESSAGE_CHECK_NO_TEXT, "mc: null text err");
	T(Message_Check_Send(NULL, &r, 0, false) == MESSAGE_CHECK_OK, "mc: null text silent");

	/* Send: argc=2, argv[1]="" → NO_TEXT */
	memset(&r, 0, sizeof(r));
	r.command = "PRIVMSG";
	r.argc = 2;
	r.argv[0] = "nick";
	r.argv[1] = "";
	T(Message_Check_Send(NULL, &r, 0, true) == MESSAGE_CHECK_NO_TEXT, "mc: empty text err");
	T(Message_Check_Send(NULL, &r, 0, false) == MESSAGE_CHECK_OK, "mc: empty text silent");

	/* Send: argc=2, argv[1]="hello" → OK */
	memset(&r, 0, sizeof(r));
	r.command = "PRIVMSG";
	r.argc = 2;
	r.argv[0] = "nick";
	r.argv[1] = "hello";
	T(Message_Check_Send(NULL, &r, 0, true) == MESSAGE_CHECK_OK, "mc: valid send ok");
	T(Message_Check_Send(NULL, &r, 0, false) == MESSAGE_CHECK_OK, "mc: valid send silent ok");

	/* Send: argc=3 → NEED_MORE_PARAMS */
	memset(&r, 0, sizeof(r));
	r.command = "PRIVMSG";
	r.argc = 3;
	r.argv[0] = "nick";
	r.argv[1] = "text";
	r.argv[2] = "extra";
	T(Message_Check_Send(NULL, &r, 0, true) == MESSAGE_CHECK_NEED_MORE_PARAMS, "mc: too many err");
	T(Message_Check_Send(NULL, &r, 0, false) == MESSAGE_CHECK_OK, "mc: too many silent");

	/* Send: argc=4 → NEED_MORE_PARAMS */
	r.argc = 4;
	T(Message_Check_Send(NULL, &r, 0, true) == MESSAGE_CHECK_NEED_MORE_PARAMS, "mc: argc4 err");

	/* Send: NOTICE command */
	memset(&r, 0, sizeof(r));
	r.command = "NOTICE";
	r.argc = 2;
	r.argv[0] = "nick";
	r.argv[1] = "text";
	T(Message_Check_Send(NULL, &r, 0, true) == MESSAGE_CHECK_OK, "mc: notice ok");
	T(Message_Check_Send(NULL, &r, 0, false) == MESSAGE_CHECK_OK, "mc: notice silent ok");

	/* Join: argc=0 → NEED_MORE_PARAMS */
	memset(&r, 0, sizeof(r));
	r.command = "JOIN";
	T(Message_Check_Join(NULL, &r) == MESSAGE_CHECK_NEED_MORE_PARAMS, "mc: join no args");

	/* Join: argc=1, argv[0]=NULL → NEED_MORE_PARAMS */
	r.argc = 1;
	r.argv[0] = NULL;
	T(Message_Check_Join(NULL, &r) == MESSAGE_CHECK_NEED_MORE_PARAMS, "mc: join null chan");

	/* Join: argc=1, argv[0]="" → NEED_MORE_PARAMS */
	r.argv[0] = "";
	T(Message_Check_Join(NULL, &r) == MESSAGE_CHECK_NEED_MORE_PARAMS, "mc: join empty chan");

	/* Join: argc=1, argv[0]="#chan" → OK */
	r.argv[0] = "#test";
	T(Message_Check_Join(NULL, &r) == MESSAGE_CHECK_OK, "mc: join valid");

	/* Join: argc=2 → OK */
	r.argc = 2;
	r.argv[0] = "#test";
	r.argv[1] = "key";
	T(Message_Check_Join(NULL, &r) == MESSAGE_CHECK_OK, "mc: join with key");

	/* Join with different channel prefixes */
	r.argc = 1;
	r.argv[0] = "&local";
	T(Message_Check_Join(NULL, &r) == MESSAGE_CHECK_OK, "mc: join local");
	r.argv[0] = "0";
	T(Message_Check_Join(NULL, &r) == MESSAGE_CHECK_OK, "mc: join zero");
}

/* =========== module_iface.c =========== */
static int plat_calls, resolve_calls, signal_calls;

static int test_plat_open(const char *p, int f, ...) { (void)p; (void)f; plat_calls++; return 0; }
static bool test_resolve_addr(PROC_STAT *s, const void *a, int i, void (*cb)(int,short))
	{ (void)s; (void)a; (void)i; (void)cb; resolve_calls++; return true; }
static bool test_resolve_name(PROC_STAT *s, const char *h, void (*cb)(int,short))
	{ (void)s; (void)h; (void)cb; resolve_calls++; return true; }
static bool test_signal_init(void) { signal_calls++; return true; }
static void test_signal_exit(void) { signal_calls++; }
static bool test_signal_notify_possible(void) { signal_calls++; return false; }
static void test_signal_notify_svc(const char *m) { (void)m; signal_calls++; }

static void test_module_iface(void)
{
	/* Initial state: all NULL */
	T(NgPlatform_GetOps() == NULL, "mi: platform null");
	T(NgResolver_GetOps() == NULL, "mi: resolver null");
	T(NgSignal_GetOps() == NULL, "mi: signal null");

	/* Platform ops */
	NgPlatformOps plat_ops = {0};
	plat_ops.open_file = test_plat_open;
	NgPlatform_SetOps(&plat_ops);
	T(NgPlatform_GetOps() == &plat_ops, "mi: platform set");
	T(NgPlatform_GetOps()->open_file != NULL, "mi: platform open");
	NgPlatform_GetOps()->open_file("test", 0);
	T(plat_calls == 1, "mi: platform dispatch");
	NgPlatform_SetOps(NULL);
	T(NgPlatform_GetOps() == NULL, "mi: platform clear");

	/* Resolver ops */
	NgResolverOps res_ops = {0};
	res_ops.resolve_addr_ident = test_resolve_addr;
	res_ops.resolve_name = test_resolve_name;
	NgResolver_SetOps(&res_ops);
	T(NgResolver_GetOps() == &res_ops, "mi: resolver set");
	NgResolver_GetOps()->resolve_addr_ident(NULL, NULL, 0, NULL);
	NgResolver_GetOps()->resolve_name(NULL, "host", NULL);
	T(resolve_calls == 2, "mi: resolver dispatch");
	NgResolver_SetOps(NULL);
	T(NgResolver_GetOps() == NULL, "mi: resolver clear");

	/* Signal ops */
	NgSignalOps sig_ops = {0};
	sig_ops.init = test_signal_init;
	sig_ops.exit = test_signal_exit;
	sig_ops.notify_possible = test_signal_notify_possible;
	sig_ops.notify_service_manager = test_signal_notify_svc;
	NgSignal_SetOps(&sig_ops);
	T(NgSignal_GetOps() == &sig_ops, "mi: signal set");
	NgSignal_GetOps()->init();
	NgSignal_GetOps()->exit();
	NgSignal_GetOps()->notify_possible();
	NgSignal_GetOps()->notify_service_manager("test");
	T(signal_calls == 4, "mi: signal dispatch");
	NgSignal_SetOps(NULL);
	T(NgSignal_GetOps() == NULL, "mi: signal clear");

	/* Log and IO (verify existing test coverage) */
	T(NgLog_GetOps() == NULL, "mi: log null");
	T(NgIo_GetOps() == NULL, "mi: io null");
}

int main(void)
{
	test_join_check();
	test_join_error();
	test_login_check();
	test_login_flow();
	test_login_state();
	test_nick_check();
	test_user_check();
	test_message_check();
	test_module_iface();

	if (failures > 0) {
		fprintf(stderr, "\n%d of %d tests FAILED\n", failures, tests);
		return 1;
	}
	printf("All %d tests passed.\n", tests);
	return 0;
}
