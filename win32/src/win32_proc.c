/*
 * ngIRCd -- The Next Generation IRC Daemon
 * Windows port: Thread-based process management (replaces fork-based proc.c)
 */

#include "portab.h"

/**
 * @file
 * Process management - Windows implementation using threads
 */

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <io.h>
#include <winsock2.h>

#include "log.h"
#include "io.h"
#include "sighandlers.h"

#include "proc.h"

/**
 * Initialize process structure.
 */
GLOBAL void
Proc_InitStruct(PROC_STAT *proc)
{
	assert(proc != NULL);
	proc->pid = 0;
	proc->pipe_fd = -1;
}

/**
 * Fork a child process - Windows stub.
 * On Windows, fork() is not available. This always returns -1 (failure).
 * Use the thread-based resolver functions in win32_resolve.c instead.
 */
GLOBAL pid_t
Proc_Fork(PROC_STAT *proc, int *pipefds, void (*cbfunc)(int, short), int timeout)
{
	(void)proc;
	(void)pipefds;
	(void)cbfunc;
	(void)timeout;

	LogDebug("Proc_Fork(): fork() not available on Windows.");
	Log(LOG_ERR, "Proc_Fork: fork() not available on Windows!");
	return -1;
}

/**
 * Generic signal handler for forked child processes - Windows stub.
 */
GLOBAL void
Proc_GenericSignalHandler(int Signal)
{
	(void)Signal;
}

/**
 * Read bytes from a pipe of a child process/thread.
 */
GLOBAL size_t
Proc_Read(PROC_STAT *proc, void *buffer, size_t buflen)
{
	ssize_t bytes_read = 0;

	assert(buffer != NULL);
	assert(buflen > 0);

	LogDebug("Proc_Read(): pipe_fd=%d, buflen=%lu.", proc ? proc->pipe_fd : -1,
	         (unsigned long)buflen);
	/* Use recv() for socket fds (loopback resolver pair), _read() for pipes */
	bytes_read = (ssize_t)recv((SOCKET)proc->pipe_fd,
				   (char *)buffer, (unsigned int)buflen, 0);
	if (bytes_read == SOCKET_ERROR) {
		int werr = WSAGetLastError();
		if (werr == WSAENOTSOCK) {
			bytes_read = (ssize_t)_read(proc->pipe_fd, buffer,
						    (unsigned int)buflen);
		} else if (werr == WSAEWOULDBLOCK) {
			return 0; /* no data yet, treat as EAGAIN */
		} else {
			errno = EIO;
			bytes_read = -1;
		}
	}
	if (bytes_read < 0) {
		if (errno == EAGAIN)
			return 0;
		Log(LOG_CRIT, "Can't read from child process %ld: %s",
		    (long)proc->pid, strerror(errno));
		Proc_Close(proc);
		bytes_read = 0;
	} else if (bytes_read == 0) {
		/* EOF: clean up */
		LogDebug("Child process %ld: EOF reached, closing pipe.",
		         (long)proc->pid);
		Proc_Close(proc);
	}
	return (size_t)bytes_read;
}

/**
 * Close pipe to a child process/thread.
 */
GLOBAL void
Proc_Close(PROC_STAT *proc)
{
	LogDebug("Proc_Close(): pipe_fd=%d.", proc ? proc->pipe_fd : -1);
	/* Close the pipe fd via the IO layer if it exists */
	if (proc->pipe_fd >= 0)
		io_close(proc->pipe_fd);

	Proc_InitStruct(proc);
}

/* -eof- */
