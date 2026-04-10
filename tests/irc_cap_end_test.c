#include "portab.h"

#include "conn.h"
#include "client.h"
#include "client-cap.h"

typedef struct _REQUEST REQUEST;
#include "irc-cap.h"
#include "login.h"

#include <stdio.h>
#include <string.h>

typedef struct TestClient {
	int type;
	int caps;
	char id[32];
} TestClient;

typedef struct _REQUEST {
	char *prefix;
	char *command;
	char *argv[15];
	int argc;
} REQUEST;

static int login_calls;
static int write_calls;
static int cap_del_calls;
static TestClient *login_client;
static TestClient *write_client;
static TestClient *capdel_client;

static int fail(const char *msg) { fprintf(stderr, "%s\n", msg); return 1; }

CONN_ID Client_Conn(CLIENT *Client) { (void)Client; return NONE; }
int Client_Type(CLIENT *Client) { return ((TestClient *)Client)->type; }
char *Client_ID(CLIENT *Client) { return ((TestClient *)Client)->id; }
char *Client_User(CLIENT *Client) { (void)Client; return (char *)"user"; }
int Client_Cap(CLIENT *Client) { return ((TestClient *)Client)->caps; }
void Client_CapSet(CLIENT *Client, int Cap) { ((TestClient *)Client)->caps = Cap; }
void Client_CapAdd(CLIENT *Client, int Cap) { ((TestClient *)Client)->caps |= Cap; }
void Client_CapDel(CLIENT *Client, int Cap) { ((TestClient *)Client)->caps &= ~Cap; cap_del_calls++; capdel_client = (TestClient *)Client; (void)Cap; }
bool IRC_WriteStrClient(CLIENT *Client, const char *Format, ...) { (void)Format; write_calls++; write_client = (TestClient *)Client; return true; }
bool IRC_WriteErrClient(CLIENT *Client, const char *Format, ...) { (void)Format; write_calls++; write_client = (TestClient *)Client; return true; }
size_t strlcpy(char *dst, const char *src, size_t siz) { size_t n = strlen(src); if (siz) { size_t c = n < siz - 1 ? n : siz - 1; memcpy(dst, src, c); dst[c] = '\0'; } return n; }
size_t strlcat(char *dst, const char *src, size_t siz) { size_t d = strlen(dst), n = strlen(src); if (d < siz) { size_t c = n < siz - d - 1 ? n : siz - d - 1; memcpy(dst + d, src, c); dst[d + c] = '\0'; } return d + n; }
void LogDebug(const char *Format, ...) { (void)Format; }
bool Login_User(CLIENT *Client) { login_calls++; login_client = (TestClient *)Client; return true; }

int main(void)
{
	TestClient c;
	REQUEST r = {0};

	memset(&c, 0, sizeof(c));
	c.type = CLIENT_GOTNICK;
	c.caps = CLIENT_CAP_PENDING;
	strcpy(c.id, "wilco");

	r.command = "CAP";
	r.argc = 1;
	r.argv[0] = "END";

	if (!IRC_CAP((CLIENT *)&c, &r))
		return fail("cap end should succeed");
	if (login_calls != 1)
		return fail("cap end should log in once");
	if (login_client != &c)
		return fail("cap end login target");
	if ((c.caps & CLIENT_CAP_PENDING) != 0)
		return fail("cap pending should clear");
	if (cap_del_calls != 1 || capdel_client != &c)
		return fail("cap pending delete tracking");
	if (write_calls != 0)
		return fail("cap end should not write anything");

	c.type = CLIENT_USER;
	c.caps = CLIENT_CAP_PENDING;
	login_calls = 0;
	write_calls = 0;
	cap_del_calls = 0;

	if (!IRC_CAP((CLIENT *)&c, &r))
		return fail("cap end registered should succeed");
	if (login_calls != 0)
		return fail("cap end on registered user should not log in");
	if (cap_del_calls != 0)
		return fail("cap end on registered user should not clear caps");

	return 0;
}
