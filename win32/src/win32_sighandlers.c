/*
 * ngIRCd -- The Next Generation IRC Daemon
 * Windows port: Simplified signal handling (replaces Unix signal-based sighandlers.c)
 */

#include "portab.h"

/**
 * @file
 * Signal Handlers - Windows implementation.
 * Uses simple signal() and a pipe for async signal delivery.
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#include "ngircd.h"
#include "sighandlers.h"

static void
Signal_Handler(int Signal)
{
	switch (Signal) {
	case SIGTERM:
	case SIGINT:
		/* Shut down server */
		NGIRCd_SignalQuit = true;
		return;
#ifdef SIGBREAK
	case SIGBREAK:
		NGIRCd_SignalQuit = true;
		return;
#endif
	}
}

/**
 * Initialize signal handlers for Windows.
 */
bool
Signals_Init(void)
{
	LogDebug("Signals_Init(): registering Windows signal handlers.");
	/* Register signal handlers */
	signal(SIGINT, Signal_Handler);
	signal(SIGTERM, Signal_Handler);
#ifdef SIGBREAK
	signal(SIGBREAK, Signal_Handler);
#endif
	return true;
}

/**
 * Restore default signal handlers.
 */
void
Signals_Exit(void)
{
	LogDebug("Signals_Exit(): restoring default signal handlers.");
	signal(SIGINT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
#ifdef SIGBREAK
	signal(SIGBREAK, SIG_DFL);
#endif
}

/**
 * Check if the service manager can be notified - not on Windows.
 */
GLOBAL bool
Signal_NotifySvcMgr_Possible(void)
{
	return false;
}

/**
 * Notify service manager - no-op on Windows.
 */
GLOBAL void
Signal_NotifySvcMgr(UNUSED const char *message)
{
	return;
}

/* -eof- */
