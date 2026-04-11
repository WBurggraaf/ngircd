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

/* Forward declaration of the original main, compiled as ngircd_main */
extern int ngircd_main(int argc, const char *argv[]);

int main(int argc, const char *argv[])
{
	WSADATA wsa;
	int ret;

	/* Initialize Winsock */
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		fprintf(stderr, "Fatal: WSAStartup failed: %d\n", WSAGetLastError());
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
