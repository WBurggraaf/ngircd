/*
 * ngIRCd -- The Next Generation IRC Daemon
 * Copyright (c)2001-2010 Alexander Barton (alex@barton.de).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * Please read the file COPYING, README and AUTHORS for more information.
 */

#ifndef __ngircd_h__
#define __ngircd_h__

/**
 * @file
 * Global variables of ngIRCd.
 */

#include <time.h>

#include "defines.h"

#define C_ARRAY_SIZE(x)	(sizeof(x)/sizeof((x)[0]))

/*
 * Per-variable dllexport/dllimport for symbols owned by ngircd_impl.dll.
 *
 * NGIRCD_IMPL_BUILD is defined when compiling sources that belong to
 * ngircd_impl.dll.  Everything else imports the symbols from the DLL.
 *
 * We use a separate NGIRCD_VAR macro rather than overriding GLOBAL, because
 * overriding GLOBAL would affect every header included after this one in the
 * same translation unit, causing C2375 "different linkage" errors on MSVC
 * when function declarations and definitions pick up conflicting attributes.
 *
 * Rules for NGIRCD_VAR:
 *   NGIRCD_IMPL_BUILD + GLOBAL_INIT  -> dllexport definition (owner TU)
 *   NGIRCD_IMPL_BUILD only           -> extern dllexport (other DLL TUs)
 *   GLOBAL_INIT only                 -> plain definition (non-DLL builds)
 *   neither                          -> extern dllimport (consumers)
 */
#if defined(_WIN32)
#  if defined(NGIRCD_IMPL_BUILD) && defined(GLOBAL_INIT)
     /* Definition exported from ngircd_impl.dll */
#    define NGIRCD_VAR            __declspec(dllexport)
#    define NGIRCD_FUNC           __declspec(dllexport)
#  elif defined(NGIRCD_IMPL_BUILD)
     /* Reference from within the same DLL */
#    define NGIRCD_VAR   extern
#    define NGIRCD_FUNC           __declspec(dllexport)
#  elif defined(NGIRCD_IMPL_CONSUMER) || defined(NGIRCD_EXE_BUILD)
     /* Import from ngircd_impl.dll */
#    define NGIRCD_VAR   extern   __declspec(dllimport)
#    define NGIRCD_FUNC           __declspec(dllimport)
#  elif defined(GLOBAL_INIT)
     /* Plain definition — static/test build */
#    define NGIRCD_VAR
#    define NGIRCD_FUNC
#  else
     /* Plain extern — static/test build reference */
#    define NGIRCD_VAR   extern
#    define NGIRCD_FUNC
#  endif
#else
#  ifdef GLOBAL_INIT
#    define NGIRCD_VAR
#    define NGIRCD_FUNC
#  else
#    define NGIRCD_VAR   extern
#    define NGIRCD_FUNC
#  endif
#endif

/** UNIX timestamp of ngIRCd start */
NGIRCD_VAR time_t NGIRCd_Start;

/** ngIRCd start time as string, used for RPL_CREATED_MSG (003) */
NGIRCD_VAR char NGIRCd_StartStr[64];

/** ngIRCd version number containing release number and compile-time options */
NGIRCD_VAR char NGIRCd_Version[126];

/** String specifying the compile-time options and target platform */
NGIRCD_VAR char NGIRCd_VersionAddition[126];

/** Flag indicating if debug mode is active (true) or not (false) */
NGIRCD_VAR bool NGIRCd_Debug;

#ifdef SNIFFER
/** Flag indication if sniffer is active (true) or not (false) */
NGIRCD_VAR bool NGIRCd_Sniffer;
#endif

/**
 * Flag indicating if NO outgoing connections should be established (true)
 * or not (false, the default)
 */
NGIRCD_VAR bool NGIRCd_Passive;

/** Flag indicating that ngIRCd has been requested to quit (true) */
NGIRCD_VAR bool NGIRCd_SignalQuit;

/** Flag indicating that ngIRCd has been requested to restart (true) */
NGIRCD_VAR bool NGIRCd_SignalRestart;

/**
 * Debug level for "VERSION" command, see description of numeric RPL_VERSION
 * (351) in RFC 2812. ngIRCd sets debuglevel to 1 when the debug mode is
 * active, and to 2 if the sniffer is running.
 */
NGIRCD_VAR char NGIRCd_DebugLevel[2];

/** Full path and file name of current configuration file */
NGIRCD_VAR char NGIRCd_ConfFile[FNAME_LEN];

/** Protocol and server identification string; see doc/Protocol.txt */
NGIRCD_VAR char NGIRCd_ProtoID[COMMAND_LEN];

NGIRCD_FUNC bool NGIRCd_Init PARAMS((bool NoDaemon));

NGIRCD_FUNC void ngircd_set_start_time PARAMS((void));

#endif

/* -eof- */
