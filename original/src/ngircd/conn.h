/*
 * ngIRCd -- The Next Generation IRC Daemon
 * Copyright (c)2001-2011 Alexander Barton (alex@barton.de) and Contributors.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * Please read the file COPYING, README and AUTHORS for more information.
 */

#ifndef __conn_h__
#define __conn_h__

/**
 * @file
 * Connection management (header)
 */

#include <time.h>			/* for time_t, see below */

/*
 * connection state flags. this is a bitmask -- all values must
 * be unique and a power of two.
 *
 * If you introduce new ones in between, make sure to adjust all
 * remaining ones.
 */
#define CONN_ISCLOSING		1	/* Conn_Close() already called */
#define CONN_ISCONNECTING	2	/* connect() in progress */
#define CONN_RFC1459		4	/* RFC 1459 compatibility mode */
#ifdef ZLIB
#define CONN_ZIP		8	/* zlib compressed link */
#endif

#include "conf-ssl.h"

#ifdef SSL_SUPPORT
#define CONN_SSL_CONNECT	16	/* wait for ssl connect to finish */
#define CONN_SSL		32	/* this connection is SSL encrypted */
#define CONN_SSL_WANT_WRITE	64	/* SSL/TLS library needs to write protocol data */
#define CONN_SSL_WANT_READ	128	/* SSL/TLS library needs to read protocol data */
#define CONN_SSL_PEERCERT_OK	256	/* peer presented a valid certificate (used to check inbound server auth */
#define CONN_SSL_FLAGS_ALL	(CONN_SSL_CONNECT|CONN_SSL|CONN_SSL_WANT_WRITE|CONN_SSL_WANT_READ|CONN_SSL_PEERCERT_OK)
#endif
typedef int CONN_ID;

#include "client.h"
#include "proc.h"

#ifdef CONN_MODULE

#ifdef NET_TRANSPORT_BUILD
# undef GLOBAL
# define GLOBAL
#endif

#include "defines.h"
#include "array.h"
#include "tool.h"
#include "ng_ipaddr.h"

#ifdef ICONV
# include <iconv.h>
#endif

#ifdef ZLIB
#include <zlib.h>
typedef struct _ZipData
{
	z_stream in;			/* "Handle" for input stream */
	z_stream out;			/* "Handle" for output stream */
	array rbuf;			/* Read buffer (compressed) */
	array wbuf;			/* Write buffer (uncompressed) */
	long bytes_in, bytes_out;	/* Counter for statistics (uncompressed!) */
} ZIPDATA;
#endif /* ZLIB */

typedef struct _Connection
{
	int sock;			/* Socket handle */
	ng_ipaddr_t addr;		/* Client address */
	PROC_STAT proc_stat;		/* Status of resolver process */
	char host[HOST_LEN];		/* Hostname */
	char *pwd;			/* password received of the client */
	array rbuf;			/* Read buffer */
	array wbuf;			/* Write buffer */
	time_t signon;			/* Signon ("connect") time */
	time_t lastdata;		/* Last activity */
	time_t lastping;		/* Last PING */
	time_t lastprivmsg;		/* Last PRIVMSG */
	time_t delaytime;		/* Ignore link ("penalty") */
	long bytes_in, bytes_out;	/* Received and sent bytes */
	long msg_in, msg_out;		/* Received and sent IRC messages */
	int flag;			/* Flag (see "irc-write" module) */
	UINT16 options;			/* Link options / connection state */
	UINT16 bps;			/* bytes processed within last second */
	CLIENT *client;			/* pointer to client structure */
#ifdef ZLIB
	ZIPDATA zip;			/* Compression information */
#endif  /* ZLIB */
#ifdef SSL_SUPPORT
	struct ConnSSL_State ssl_state;	/* SSL/GNUTLS state information */
#endif
#ifndef STRICT_RFC
	long auth_ping;			/** PING response expected on login */
#endif
#ifdef ICONV
	iconv_t iconv_from;		/** iconv: convert from client to server */
	iconv_t iconv_to;		/** iconv: convert from server to client */
#endif
} CONNECTION;


#ifdef CONN_MODULE_GLOBAL_INIT
CONNECTION *My_Connections;
CONN_ID Pool_Size;
long WCounter;
#else
extern CONNECTION *My_Connections;
extern CONN_ID Pool_Size;
extern long WCounter;
#endif


#define CONNECTION2ID(x) (long)(x - My_Connections)

#endif /* CONN_MODULE */


#ifdef NET_TRANSPORT_BUILD
# define CONN_API extern
#else
# define CONN_API GLOBAL
#endif

CONN_API void Conn_Init PARAMS((void ));
CONN_API void Conn_Exit PARAMS(( void ));

CONN_API void Conn_CloseAllSockets PARAMS((int ExceptOf));

CONN_API unsigned int Conn_InitListeners PARAMS(( void ));
CONN_API void Conn_ExitListeners PARAMS(( void ));

CONN_API void Conn_StartLogin PARAMS((CONN_ID Idx));

CONN_API void Conn_Handler PARAMS(( void ));

CONN_API bool Conn_WriteStr PARAMS(( CONN_ID Idx, const char *Format, ... ));
CONN_API bool Conn_WriteStrBuf PARAMS(( CONN_ID Idx, const char *Str ));

CONN_API char* Conn_Password PARAMS(( CONN_ID Idx ));
CONN_API void Conn_SetPassword PARAMS(( CONN_ID Idx, const char *Pwd ));

CONN_API void Conn_Close PARAMS(( CONN_ID Idx, const char *LogMsg, const char *FwdMsg, bool InformClient ));

CONN_API void Conn_SyncServerStruct PARAMS(( void ));

CONN_API CONN_ID Conn_GetFromProc PARAMS((int fd));
CONN_API CLIENT* Conn_GetClient PARAMS((CONN_ID i));
CONN_API PROC_STAT* Conn_GetProcStat PARAMS((CONN_ID i));

CONN_API char *Conn_GetCertFp PARAMS((CONN_ID Idx));
CONN_API bool Conn_SetCertFp PARAMS((CONN_ID Idx, const char *fingerprint));
CONN_API bool Conn_UsesSSL PARAMS((CONN_ID Idx));

#ifdef SSL_SUPPORT
CONN_API bool Conn_GetCipherInfo PARAMS((CONN_ID Idx, char *buf, size_t len));
#endif

CONN_API const char *Conn_GetIPAInfo PARAMS((CONN_ID Idx));

CONN_API long Conn_Count PARAMS((void));
CONN_API long Conn_CountMax PARAMS((void));
CONN_API long Conn_CountAccepted PARAMS((void));

#ifndef STRICT_RFC
CONN_API long Conn_GetAuthPing PARAMS((CONN_ID Idx));
CONN_API void Conn_SetAuthPing PARAMS((CONN_ID Idx, long ID));
#endif

CONN_API void Conn_DebugDump PARAMS((void));

#endif

/* -eof- */
