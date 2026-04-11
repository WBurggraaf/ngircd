/*
 * conf_func_stubs.c — minimal stubs for Conf_Init / Conf_Rehash / Conf_Test
 *
 * Used by config_module_test to satisfy link-time references from config.c
 * without pulling in the full conf.c implementation.
 *
 * The Conf_* global variables are supplied by test_support.c.
 */

#include "portab/portab.h"

/* Declare the functions to match the signatures in conf.h */
void Conf_Init(void);
bool Conf_Rehash(void);
int  Conf_Test(void);

void Conf_Init(void)          {}
bool Conf_Rehash(void)        { return 1; }
int  Conf_Test(void)          { return 0; }

/* Conf_Test_ValidateConfig is referenced by some conf.h consumers */
void Conf_Test_SetDefaults(bool InitServers) { (void)InitServers; }
bool Conf_Test_ValidateConfig(bool TestOnly, bool Rehash)
{ (void)TestOnly; (void)Rehash; return 1; }
