/*
 * logging_module_test.c — unit tests for the managed Logging.dll API
 *
 * Links against the managed NativeAOT Logging.dll (C# implementation).
 * No stub functions needed — the managed DLL has its own real implementation.
 *
 * Tests:
 *   - logging_get_api_v1()   struct integrity (all function pointers present)
 *   - module_get_api_v1()    metadata integrity
 *   - init/shutdown/reinit   lifecycle smoke
 *   - log_message / debug_message / fatal_message  smoke (no crash)
 *   - ngircd-root.log        created by managed logging on first write
 */

#include "app/logging/logging.h"
#include "core/runtime/module_api.h"

#include <stdio.h>
#include <stdlib.h>

/* Forward-declare module_get_api_v1 — exported by logging.dll */
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
    CHECK(api != NULL,                   "logging_api_not_null");
    CHECK(api->api_major == 1u,          "logging_api_major_1");
    CHECK(api->api_minor == 0u,          "logging_api_minor_0");
    CHECK(api->init          != NULL,    "logging_init_ptr_not_null");
    CHECK(api->shutdown      != NULL,    "logging_shutdown_ptr_not_null");
    CHECK(api->reinit        != NULL,    "logging_reinit_ptr_not_null");
    CHECK(api->log_message   != NULL,    "logging_log_message_ptr_not_null");
    CHECK(api->debug_message != NULL,    "logging_debug_message_ptr_not_null");
    CHECK(api->fatal_message != NULL,    "logging_fatal_message_ptr_not_null");

    /* ---- Idempotent getter ---- */
    CHECK(logging_get_api_v1() == api,   "logging_api_idempotent");

    /* ---- Lifecycle smoke ---- */
    CHECK(api->init(0) == CORE_STATUS_OK, "logging_init_returns_ok");
    api->reinit();
    api->shutdown();

    /* ---- Message smoke (must not crash) ---- */
    api->log_message(6, "test log_message");
    api->debug_message("test debug_message");
    api->fatal_message("test fatal_message");

    /* ---- Managed DLL writes ngircd-root.log in working directory ---- */
    {
        FILE *fp = fopen("ngircd-root.log", "rb");
        CHECK(fp != NULL, "logging_root_log_exists");
        fclose(fp);
    }

    /* ---- module_get_api_v1 metadata ---- */
    const core_module_api_t *m = module_get_api_v1();
    CHECK(m != NULL,                                           "logging_mod_api_not_null");
    CHECK(m->api_major == 1u,                                  "logging_mod_api_major_1");
    CHECK(m->api_minor == 0u,                                  "logging_mod_api_minor_0");
    CHECK(m->metadata != NULL,                                 "logging_mod_metadata_not_null");
    CHECK(m->metadata->module_kind == CORE_MODULE_KIND_LOGGING,"logging_mod_kind_correct");
    CHECK(m->metadata->module_name != NULL,                    "logging_mod_name_not_null");
    CHECK(m->metadata->module_name[0] != '\0',                 "logging_mod_name_nonempty");
    CHECK(m->api != NULL,                                      "logging_mod_api_ptr_not_null");
    CHECK(m->api == (const void *)logging_get_api_v1(),        "logging_mod_api_matches_typed");

    printf("logging_module_test: all 21 checks passed\n");
    return 0;
}
