/*
 * ngircd_runtime_globals.c
 *
 * Provides the authoritative definitions of all NGIRCd_* global variables
 * for ngircd_impl.dll.  Uses the GLOBAL_INIT + NGIRCD_IMPL_BUILD trick so
 * that portab.h/ngircd.h expand GLOBAL to __declspec(dllexport), making
 * every variable visible to the EXE and other DLLs that import ngircd_impl.
 */

#define NGIRCD_IMPL_BUILD
#define GLOBAL_INIT
#include "portab.h"
#include "ngircd.h"
/* Defines all NGIRCd_* globals and exports them from ngircd_impl.dll */
