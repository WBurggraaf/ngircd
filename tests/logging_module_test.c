/*
 * logging_module_test.c — unit tests for the logging wrapper DLL API
 *
 * Compiles src/app/logging/logging.c directly with LOGGING_BUILD.
 * Provides no-op stubs for Log_*, so no ngircd_impl link needed.
 *
 * Tests:
 *   - logging_get_api_v1()   struct integrity
 *   - module_get_api_v1()    metadata integrity
 *   - logging_log/debug smoke (exercise code paths)
 */

#include "app/logging/logging.h"
#include "core/runtime/module_api.h"

#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>

/* ---- Stubs for Log_* functions called from logging.c ---- */

void Log_Init(bool syslog_mode) { (void)syslog_mode; }
void Log_Exit(void) {}
void Log_ReInit(void) {}
void Log(int level, const char *format, ...) { (void)level; (void)format; }
void LogDebug(const char *format, ...) { (void)format; }

/* Forward-declare module_get_api_v1 compiled from logging.c */
extern const core_module_api_t * module_get_api_v1(void);

/* ---- CHECK macro ---- */
#define CHECK(expr, name) \
    do { \
        if (!(expr)) { \
            fprintf(stderr, "FAIL: %s\n", (name)); \
            return 1; \
        } \
    } while (0)

int main(void)
{
    /* ---- logging_get_api_v1 struct ---- */
    const logging_api_t *api = logging_get_api_v1();
    CHECK(api != NULL,                  "logging_api_not_null");
    CHECK(api->api_major == 1u,         "logging_api_major_1");
    CHECK(api->api_minor == 0u,         "logging_api_minor_0");
    CHECK(api->init     != NULL,        "logging_init_ptr_not_null");
    CHECK(api->shutdown != NULL,        "logging_shutdown_ptr_not_null");
    CHECK(api->reinit   != NULL,        "logging_reinit_ptr_not_null");
    CHECK(api->log_message   != NULL,   "logging_log_ptr_not_null");
    CHECK(api->debug_message != NULL,   "logging_debug_ptr_not_null");

    /* ---- Smoke: exercise init/reinit/shutdown code paths ---- */
    CHECK(api->init(0)  == CORE_STATUS_OK, "logging_init_returns_ok");
    api->reinit();   /* no-op stub path */
    api->shutdown(); /* no-op stub path */

    /* ---- Smoke: log and debug should not crash ---- */
    api->log_message(0, "test 42");
    api->debug_message("dbg hello");

    /* ---- module_get_api_v1 metadata ---- */
    const core_module_api_t *m = module_get_api_v1();
    CHECK(m != NULL,                                        "logging_mod_api_not_null");
    CHECK(m->api_major == 1u,                              "logging_mod_api_major_1");
    CHECK(m->api_minor == 0u,                              "logging_mod_api_minor_0");
    CHECK(m->metadata != NULL,                             "logging_mod_metadata_not_null");
    CHECK(m->metadata->module_kind == CORE_MODULE_KIND_LOGGING, "logging_mod_kind_correct");
    CHECK(m->metadata->module_name != NULL,                "logging_mod_name_not_null");
    CHECK(m->metadata->module_name[0] != '\0',             "logging_mod_name_nonempty");
    CHECK(m->api != NULL,                                  "logging_mod_api_ptr_not_null");
    /* api pointer must be the same as the typed getter */
    CHECK(m->api == (const void *)logging_get_api_v1(),    "logging_mod_api_matches_typed");

    printf("logging_module_test: all 20 checks passed\n");
    return 0;
}

