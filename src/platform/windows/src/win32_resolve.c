/*
 * ngIRCd -- The Next Generation IRC Daemon
 * Windows port: pipe-based resolver callback via loopback socket pair.
 *
 * Winsock select() only accepts socket handles, not pipe fds.
 * We create a connected loopback TCP socket pair, write the resolver
 * result ("\n" = empty hostname, matching the DNS=no path in the Unix
 * resolver) to the write end, then register the read end with the IO
 * subsystem so cb_Read_Resolver_Result fires normally.
 */

#include "portab.h"

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <winsock2.h>
#include <ws2tcpip.h>

#include "conn.h"
#include "conf.h"
#include "log.h"
#include "ng_ipaddr.h"
#include "io.h"
#include "proc.h"

#include "resolve.h"

/**
 * Create a connected loopback TCP socket pair.
 * *rd receives the read end (to register with IO), *wr the write end.
 * Returns true on success.
 */
static bool
make_loopback_socketpair(SOCKET *rd, SOCKET *wr)
{
	SOCKET listener, connector, accepted;
	struct sockaddr_in addr;
	int addrlen = sizeof(addr);

	listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listener == INVALID_SOCKET)
		return false;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_port = 0;

	if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) != 0 ||
	    listen(listener, 1) != 0 ||
	    getsockname(listener, (struct sockaddr *)&addr, &addrlen) != 0) {
		closesocket(listener);
		return false;
	}

	connector = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connector == INVALID_SOCKET) {
		closesocket(listener);
		return false;
	}

	if (connect(connector, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
		closesocket(connector);
		closesocket(listener);
		return false;
	}

	accepted = accept(listener, NULL, NULL);
	closesocket(listener);

	if (accepted == INVALID_SOCKET) {
		closesocket(connector);
		return false;
	}

	*rd = accepted;
	*wr = connector;
	return true;
}

/**
 * Resolve IP address for an incoming connection.
 *
 * With DNS disabled (the only supported mode on Windows) the original Unix
 * resolver writes an empty hostname followed by '\n' into the pipe and exits.
 * We replicate that exactly via a loopback socket pair so the IO subsystem
 * (Winsock select) can monitor the read end.
 */
GLOBAL bool
Resolve_Addr_Ident(PROC_STAT *s, const ng_ipaddr_t *Addr, int identsock,
		   void (*cbfunc)(int, short))
{
	SOCKET rd, wr;

	(void)identsock; /* IDENT not supported on Windows */

	assert(s != NULL);

	if (!make_loopback_socketpair(&rd, &wr)) {
		Log(LOG_ALERT, "Resolve_Addr_Ident: socketpair failed: %d",
		    WSAGetLastError());
		return false;
	}

	/* Write the DNS=no result: empty hostname + '\n' */
	if (send(wr, "\n", 1, 0) != 1) {
		Log(LOG_CRIT, "Resolve_Addr_Ident: send() failed: %d",
		    WSAGetLastError());
		closesocket(rd);
		closesocket(wr);
		return false;
	}
	closesocket(wr); /* EOF on read end after this */

	if (!io_setnonblock((int)rd)) {
		Log(LOG_CRIT, "Resolve_Addr_Ident: io_setnonblock() failed");
		closesocket(rd);
		return false;
	}

	if (!io_event_create((int)rd, IO_WANTREAD, cbfunc)) {
		Log(LOG_CRIT, "Resolve_Addr_Ident: io_event_create() failed: %d",
		    WSAGetLastError());
		closesocket(rd);
		return false;
	}

	/* Mark as in-progress so Proc_InProgress() and Conn_GetFromProc() work. */
	s->pid = 1;
	s->pipe_fd = (int)rd;

	LogDebug("Resolve_Addr_Ident(): loopback socket pair registered for %s (fd=%d).",
		 ng_ipaddr_tostr(Addr), (int)rd);
	return true;
}

/**
 * Resolve hostname for outgoing server connections.
 * Not currently implemented on Windows (no server links).
 */
GLOBAL bool
Resolve_Name(PROC_STAT *s, const char *Host, void (*cbfunc)(int, short))
{
	(void)s;
	(void)cbfunc;
	assert(Host != NULL);
	LogDebug("Resolve_Name(): not implemented on Windows (host=%s).", Host);
	return false;
}

/* -eof- */
