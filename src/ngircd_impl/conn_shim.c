/*
 * conn_shim.c  —  runtime forwarding shim for connection functions
 *
 * Phase 2 compatibility bridge: conn.c (and conn-func.c, conn-encoding.c,
 * io.c) have been moved from ngircd_impl into net_transport.dll.
 *
 * ngircd_impl's proto and core files still call Conn_*/io_* by direct name.
 * This shim provides those symbols inside ngircd_impl.dll by loading
 * net_transport.dll at runtime (via LoadLibraryA) and forwarding every call
 * through GetProcAddress-resolved function pointers.
 *
 * No link-time dependency on net_transport is needed, so there is no circular
 * CMake dependency.  net_transport.dll itself imports from ngircd_impl.dll for
 * Client_*, Conf_*, Log_* etc., but that direction already worked before.
 *
 * Initialisation:
 *   ConnShim_Init()  must be called once, before any Conn_* or io_* function
 *   is used.  It is called from NGIRCd_Init() in ngircd_daemon.c.
 */

#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "portab.h"
#include "conn.h"
#include "conn-func.h"
#include "conn-encoding.h"
#include "io.h"
#include "proc.h"

/* Maximum IRC command length (matches conn.c / RFC 2812) */
#ifndef COMMAND_LEN
# define COMMAND_LEN 512
#endif

/* ------------------------------------------------------------------ */
/* Internal state                                                       */
/* ------------------------------------------------------------------ */

static HMODULE s_hNetTransport = NULL;

/* Helper: load net_transport.dll if not already loaded. */
static void
ConnShim_EnsureLoaded(void)
{
#ifdef _WIN32
	if (!s_hNetTransport) {
		s_hNetTransport = LoadLibraryA("net_transport.dll");
		assert(s_hNetTransport != NULL);
	}
#endif
}

/* Helper: resolve a function pointer from net_transport.dll */
static FARPROC
ConnShim_Resolve(const char *name)
{
	FARPROC p;
	ConnShim_EnsureLoaded();
	p = GetProcAddress(s_hNetTransport, name);
	assert(p != NULL);
	return p;
}

/* Macro: declare a cached function-pointer slot + lazy resolver */
#define DECLARE_FN(ret, name, params)               \
	typedef ret (*_fp_##name##_t) params;           \
	static _fp_##name##_t _fp_##name = NULL;        \
	static _fp_##name##_t _get_##name(void) {       \
		if (!_fp_##name)                             \
			_fp_##name = (_fp_##name##_t)(void *)ConnShim_Resolve(#name); \
		return _fp_##name;                           \
	}

/* ------------------------------------------------------------------ */
/* Public init / shutdown                                               */
/* ------------------------------------------------------------------ */

/**
 * Pre-warm all function-pointer slots from net_transport.dll.
 * Called from NGIRCd_Init() before any connection activity starts.
 */
void
ConnShim_Init(void)
{
#ifdef _WIN32
	ConnShim_EnsureLoaded();
#endif
}

void
ConnShim_Exit(void)
{
#ifdef _WIN32
	if (s_hNetTransport) {
		FreeLibrary(s_hNetTransport);
		s_hNetTransport = NULL;
	}
#endif
}

/* ------------------------------------------------------------------ */
/* conn.h forwarding stubs                                              */
/* ------------------------------------------------------------------ */

DECLARE_FN(void,  Conn_Init,             (void))
DECLARE_FN(void,  Conn_Exit,             (void))
DECLARE_FN(void,  Conn_CloseAllSockets,  (int))
DECLARE_FN(unsigned int, Conn_InitListeners, (void))
DECLARE_FN(void,  Conn_ExitListeners,    (void))
DECLARE_FN(void,  Conn_StartLogin,       (CONN_ID))
DECLARE_FN(void,  Conn_Handler,          (void))
DECLARE_FN(bool,  Conn_WriteStrBuf,      (CONN_ID, const char *))
DECLARE_FN(char*, Conn_Password,         (CONN_ID))
DECLARE_FN(void,  Conn_SetPassword,      (CONN_ID, const char *))
DECLARE_FN(void,  Conn_Close,            (CONN_ID, const char *, const char *, bool))
DECLARE_FN(void,  Conn_SyncServerStruct, (void))
DECLARE_FN(CONN_ID, Conn_GetFromProc,    (int))
DECLARE_FN(CLIENT*, Conn_GetClient,      (CONN_ID))
DECLARE_FN(PROC_STAT*, Conn_GetProcStat, (CONN_ID))
DECLARE_FN(char*, Conn_GetCertFp,        (CONN_ID))
DECLARE_FN(bool,  Conn_SetCertFp,        (CONN_ID, const char *))
DECLARE_FN(bool,  Conn_UsesSSL,          (CONN_ID))
DECLARE_FN(bool,  Conn_GetCipherInfo,    (CONN_ID, char *, size_t))
DECLARE_FN(const char*, Conn_GetIPAInfo, (CONN_ID))
DECLARE_FN(long,  Conn_Count,            (void))
DECLARE_FN(long,  Conn_CountMax,         (void))
DECLARE_FN(long,  Conn_CountAccepted,    (void))
DECLARE_FN(long,  Conn_GetAuthPing,      (CONN_ID))
DECLARE_FN(void,  Conn_SetAuthPing,      (CONN_ID, long))
DECLARE_FN(void,  Conn_DebugDump,        (void))

void  Conn_Init(void)                         { _get_Conn_Init()(); }
void  Conn_Exit(void)                         { _get_Conn_Exit()(); }
void  Conn_CloseAllSockets(int e)             { _get_Conn_CloseAllSockets()(e); }
unsigned int Conn_InitListeners(void)         { return _get_Conn_InitListeners()(); }
void  Conn_ExitListeners(void)                { _get_Conn_ExitListeners()(); }
void  Conn_StartLogin(CONN_ID i)              { _get_Conn_StartLogin()(i); }
void  Conn_Handler(void)                      { _get_Conn_Handler()(); }

/** Variadic write: format here, forward pre-formatted string */
bool  Conn_WriteStr(CONN_ID Idx, const char *Format, ...)
{
	char buf[COMMAND_LEN];
	va_list ap;
	va_start(ap, Format);
	vsnprintf(buf, sizeof(buf) - 2, Format, ap);
	va_end(ap);
	return _get_Conn_WriteStrBuf()(Idx, buf);
}

char* Conn_Password(CONN_ID i)                { return _get_Conn_Password()(i); }
void  Conn_SetPassword(CONN_ID i, const char *p) { _get_Conn_SetPassword()(i, p); }
void  Conn_Close(CONN_ID i, const char *l, const char *f, bool ic)
                                              { _get_Conn_Close()(i, l, f, ic); }
void  Conn_SyncServerStruct(void)             { _get_Conn_SyncServerStruct()(); }
CONN_ID Conn_GetFromProc(int fd)              { return _get_Conn_GetFromProc()(fd); }
CLIENT* Conn_GetClient(CONN_ID i)             { return _get_Conn_GetClient()(i); }
PROC_STAT* Conn_GetProcStat(CONN_ID i)        { return _get_Conn_GetProcStat()(i); }
char* Conn_GetCertFp(CONN_ID i)               { return _get_Conn_GetCertFp()(i); }
bool  Conn_SetCertFp(CONN_ID i, const char *fp) { return _get_Conn_SetCertFp()(i, fp); }
bool  Conn_UsesSSL(CONN_ID i)                 { return _get_Conn_UsesSSL()(i); }
bool  Conn_GetCipherInfo(CONN_ID i, char *b, size_t l) { return _get_Conn_GetCipherInfo()(i, b, l); }
const char* Conn_GetIPAInfo(CONN_ID i)        { return _get_Conn_GetIPAInfo()(i); }
long  Conn_Count(void)                        { return _get_Conn_Count()(); }
long  Conn_CountMax(void)                     { return _get_Conn_CountMax()(); }
long  Conn_CountAccepted(void)                { return _get_Conn_CountAccepted()(); }
long  Conn_GetAuthPing(CONN_ID i)             { return _get_Conn_GetAuthPing()(i); }
void  Conn_SetAuthPing(CONN_ID i, long id)    { _get_Conn_SetAuthPing()(i, id); }
void  Conn_DebugDump(void)                    { _get_Conn_DebugDump()(); }

/* ------------------------------------------------------------------ */
/* conn-func.h forwarding stubs                                         */
/* ------------------------------------------------------------------ */

DECLARE_FN(void,   Conn_UpdateIdle,   (CONN_ID))
DECLARE_FN(void,   Conn_UpdatePing,   (CONN_ID, time_t))
DECLARE_FN(time_t, Conn_GetSignon,    (CONN_ID))
DECLARE_FN(time_t, Conn_GetIdle,      (CONN_ID))
DECLARE_FN(time_t, Conn_LastPing,     (CONN_ID))
DECLARE_FN(time_t, Conn_StartTime,    (CONN_ID))
DECLARE_FN(size_t, Conn_SendQ,        (CONN_ID))
DECLARE_FN(size_t, Conn_RecvQ,        (CONN_ID))
DECLARE_FN(long,   Conn_SendMsg,      (CONN_ID))
DECLARE_FN(long,   Conn_RecvMsg,      (CONN_ID))
DECLARE_FN(long,   Conn_SendBytes,    (CONN_ID))
DECLARE_FN(long,   Conn_RecvBytes,    (CONN_ID))
DECLARE_FN(const char*, Conn_IPA,     (CONN_ID))
DECLARE_FN(void,   Conn_SetPenalty,   (CONN_ID, time_t))
DECLARE_FN(void,   Conn_ClearFlags,   (void))
DECLARE_FN(int,    Conn_Flag,         (CONN_ID))
DECLARE_FN(void,   Conn_SetFlag,      (CONN_ID, int))
DECLARE_FN(CONN_ID, Conn_First,       (void))
DECLARE_FN(CONN_ID, Conn_Next,        (CONN_ID))
DECLARE_FN(UINT16, Conn_Options,      (CONN_ID))
DECLARE_FN(void,   Conn_SetOption,    (CONN_ID, int))
DECLARE_FN(void,   Conn_ResetWCounter,(void))
DECLARE_FN(long,   Conn_WCounter,     (void))

void   Conn_UpdateIdle(CONN_ID i)         { _get_Conn_UpdateIdle()(i); }
void   Conn_UpdatePing(CONN_ID i, time_t t) { _get_Conn_UpdatePing()(i, t); }
time_t Conn_GetSignon(CONN_ID i)          { return _get_Conn_GetSignon()(i); }
time_t Conn_GetIdle(CONN_ID i)            { return _get_Conn_GetIdle()(i); }
time_t Conn_LastPing(CONN_ID i)           { return _get_Conn_LastPing()(i); }
time_t Conn_StartTime(CONN_ID i)          { return _get_Conn_StartTime()(i); }
size_t Conn_SendQ(CONN_ID i)              { return _get_Conn_SendQ()(i); }
size_t Conn_RecvQ(CONN_ID i)              { return _get_Conn_RecvQ()(i); }
long   Conn_SendMsg(CONN_ID i)            { return _get_Conn_SendMsg()(i); }
long   Conn_RecvMsg(CONN_ID i)            { return _get_Conn_RecvMsg()(i); }
long   Conn_SendBytes(CONN_ID i)          { return _get_Conn_SendBytes()(i); }
long   Conn_RecvBytes(CONN_ID i)          { return _get_Conn_RecvBytes()(i); }
const char* Conn_IPA(CONN_ID i)          { return _get_Conn_IPA()(i); }
void   Conn_SetPenalty(CONN_ID i, time_t s) { _get_Conn_SetPenalty()(i, s); }
void   Conn_ClearFlags(void)              { _get_Conn_ClearFlags()(); }
int    Conn_Flag(CONN_ID i)               { return _get_Conn_Flag()(i); }
void   Conn_SetFlag(CONN_ID i, int f)     { _get_Conn_SetFlag()(i, f); }
CONN_ID Conn_First(void)                  { return _get_Conn_First()(); }
CONN_ID Conn_Next(CONN_ID i)              { return _get_Conn_Next()(i); }
UINT16 Conn_Options(CONN_ID i)            { return _get_Conn_Options()(i); }
void   Conn_SetOption(CONN_ID i, int o)   { _get_Conn_SetOption()(i, o); }
void   Conn_ResetWCounter(void)           { _get_Conn_ResetWCounter()(); }
long   Conn_WCounter(void)                { return _get_Conn_WCounter()(); }

/* ------------------------------------------------------------------ */
/* conn-encoding.h forwarding stubs                                     */
/* ------------------------------------------------------------------ */

DECLARE_FN(bool,  Conn_SetEncoding,   (CONN_ID, const char *))
DECLARE_FN(void,  Conn_UnsetEncoding, (CONN_ID))
DECLARE_FN(char*, Conn_EncodingFrom,  (CONN_ID, char *))
DECLARE_FN(char*, Conn_EncodingTo,    (CONN_ID, char *))

bool  Conn_SetEncoding(CONN_ID i, const char *e)  { return _get_Conn_SetEncoding()(i, e); }
void  Conn_UnsetEncoding(CONN_ID i)               { _get_Conn_UnsetEncoding()(i); }
char* Conn_EncodingFrom(CONN_ID i, char *m)       { return _get_Conn_EncodingFrom()(i, m); }
char* Conn_EncodingTo(CONN_ID i, char *m)         { return _get_Conn_EncodingTo()(i, m); }

/* ------------------------------------------------------------------ */
/* io.h forwarding stubs                                                */
/* io_* functions are called by conn.c which now lives in net_transport */
/* but may also be referenced from ngircd_impl's win32_* wrappers.     */
/* ------------------------------------------------------------------ */

DECLARE_FN(bool, io_library_init,     (unsigned int))
DECLARE_FN(void, io_library_shutdown, (void))
DECLARE_FN(bool, io_event_create,     (int, short, void (*)(int, short)))
DECLARE_FN(bool, io_event_setcb,      (int, void (*)(int, short)))
DECLARE_FN(bool, io_event_add,        (int, short))
DECLARE_FN(bool, io_event_del,        (int, short))
DECLARE_FN(bool, io_close,            (int))
DECLARE_FN(bool, io_setnonblock,      (int))
DECLARE_FN(bool, io_setcloexec,       (int))
DECLARE_FN(int,  io_dispatch,         (struct timeval *))

bool io_library_init(unsigned int n)              { return _get_io_library_init()(n); }
void io_library_shutdown(void)                    { _get_io_library_shutdown()(); }
bool io_event_create(int fd, short w, void (*cb)(int,short))
                                                  { return _get_io_event_create()(fd,w,cb); }
bool io_event_setcb(int fd, void (*cb)(int,short)){ return _get_io_event_setcb()(fd,cb); }
bool io_event_add(int fd, short w)                { return _get_io_event_add()(fd,w); }
bool io_event_del(int fd, short w)                { return _get_io_event_del()(fd,w); }
bool io_close(int fd)                             { return _get_io_close()(fd); }
bool io_setnonblock(int fd)                       { return _get_io_setnonblock()(fd); }
bool io_setcloexec(int fd)                        { return _get_io_setcloexec()(fd); }
int  io_dispatch(struct timeval *tv)              { return _get_io_dispatch()(tv); }
