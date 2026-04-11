/*
 * platform_module_test.c — unit tests for the managed Platform.dll API
 *
 * Links against the managed NativeAOT Platform.dll (C# implementation).
 * No stubs or portab needed — the managed DLL is self-contained.
 *
 * Tests:
 *   - platform_get_api_v1()  struct integrity (all function pointers present)
 *   - module_get_api_v1()    metadata integrity
 *   - close_file(-1)         error path returns IO_ERROR
 *   - open_file(NULL, 0)     error path returns IO_ERROR
 */

#include "platform/platform.h"
#include "core/runtime/module_api.h"

#include <stdio.h>

/* Forward-declare module_get_api_v1 exported by Platform.dll */
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
    /* ---- platform_get_api_v1 struct ---- */
    const platform_api_t *api = platform_get_api_v1();
    CHECK(api != NULL,                   "platform_api_not_null");
    CHECK(api->api_major == 1u,          "platform_api_major_1");
    CHECK(api->api_minor == 0u,          "platform_api_minor_0");
    CHECK(api->open_file    != NULL,     "platform_open_file_ptr_not_null");
    CHECK(api->close_file   != NULL,     "platform_close_file_ptr_not_null");
    CHECK(api->set_nonblock != NULL,     "platform_set_nonblock_ptr_not_null");
    CHECK(api->set_cloexec  != NULL,     "platform_set_cloexec_ptr_not_null");

    /* ---- Idempotent getter ---- */
    CHECK(platform_get_api_v1() == api,  "platform_api_idempotent");

    /* ---- Error-path: close invalid fd → IO_ERROR ---- */
    CHECK(api->close_file(-1) == CORE_STATUS_IO_ERROR, "platform_close_invalid_fd");

    /* ---- Error-path: open NULL path → IO_ERROR ---- */
    CHECK(api->open_file(NULL, 0) == CORE_STATUS_IO_ERROR, "platform_open_null_path");

    /* ---- module_get_api_v1 metadata ---- */
    const core_module_api_t *m = module_get_api_v1();
    CHECK(m != NULL,                                             "platform_mod_api_not_null");
    CHECK(m->api_major == 1u,                                    "platform_mod_api_major_1");
    CHECK(m->api_minor == 0u,                                    "platform_mod_api_minor_0");
    CHECK(m->metadata != NULL,                                   "platform_mod_metadata_not_null");
    CHECK(m->metadata->module_kind == CORE_MODULE_KIND_PLATFORM, "platform_mod_kind_correct");
    CHECK(m->metadata->module_name != NULL,                      "platform_mod_name_not_null");
    CHECK(m->metadata->module_name[0] != '\0',                   "platform_mod_name_nonempty");
    CHECK(m->api != NULL,                                        "platform_mod_api_ptr_not_null");
    CHECK(m->api == (const void *)platform_get_api_v1(),         "platform_mod_api_matches_typed");

    printf("platform_module_test: all 19 checks passed\n");
    return 0;
}
