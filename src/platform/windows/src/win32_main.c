/*
 * ngIRCd -- The Next Generation IRC Daemon
 * Windows port: Winsock initialization wrapper
 *
 * This file wraps the original main() to add Winsock startup/cleanup.
 * The original main() is renamed to ngircd_main() via preprocessor.
 */

#include "portab.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/* Forward declaration of the original main, compiled as ngircd_main */
extern int ngircd_main(int argc, const char *argv[]);

typedef struct logging_api {
	void *reserved[2];
	void (*init)(int syslog_mode);
	void (*shutdown)(void);
	void (*reinit)(void);
	void (*log_message)(int level, const char *message);
	void (*debug_message)(const char *message);
	void (*fatal_message)(const char *message);
} logging_api_t;

static void
bootstrap_log_fatal(const char *fmt, ...)
{
	char msg[1024];
	char line[1200];
	va_list ap;
	HMODULE h;
	typedef const logging_api_t * (__cdecl *logging_get_api_v1_fn)(void);
	logging_get_api_v1_fn get_api;
	const logging_api_t *api = NULL;

	va_start(ap, fmt);
	vsnprintf(msg, sizeof msg, fmt, ap);
	va_end(ap);

	h = LoadLibraryA("logging.dll");
	if (h) {
		get_api = (logging_get_api_v1_fn)(void *)GetProcAddress(h, "logging_get_api_v1");
		if (get_api)
			api = get_api();
	}

	if (api && api->fatal_message) {
		api->fatal_message(msg);
		return;
	}

	snprintf(line, sizeof line, "%s\n", msg);
	fprintf(stderr, "%s", line);
}

int main(int argc, const char *argv[])
{
	WSADATA wsa;
	int ret;

	/* Initialize Winsock */
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		bootstrap_log_fatal("Fatal: WSAStartup failed: %d", WSAGetLastError());
		return 1;
	}

	/* Force nodaemon mode on Windows by injecting --nodaemon if not present */
	{
		int i;
		bool has_nodaemon = false;
		for (i = 1; i < argc; i++) {
			if (strcmp(argv[i], "--nodaemon") == 0 || strcmp(argv[i], "-n") == 0) {
				has_nodaemon = true;
				break;
			}
		}
		if (!has_nodaemon) {
			/* Add --nodaemon to argv */
			const char **new_argv = (const char **)malloc((argc + 2) * sizeof(char *));
			if (new_argv) {
				for (i = 0; i < argc; i++)
					new_argv[i] = argv[i];
				new_argv[argc] = "--nodaemon";
				new_argv[argc + 1] = NULL;
				argc++;
				argv = new_argv;
			}
		}
	}

	ret = ngircd_main(argc, argv);

	WSACleanup();
	return ret;
}

/* -eof- */
