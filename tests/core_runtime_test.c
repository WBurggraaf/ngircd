/*
 * core_runtime_test.c — unit tests for core_runtime module
 *
 * Tests core_runtime_api_version, status helpers, version_make,
 * and the module_kind constant set.
 *
 * Compiles core_runtime.c directly; no DLL deps needed.
 */

#include "core/runtime/core_runtime.h"
#include "core/runtime/module_api.h"

#include <stdio.h>

#define CHECK(expr, name) \
    do { \
        if (!(expr)) { \
            fprintf(stderr, "FAIL: %s\n", (name)); \
            return 1; \
        } \
    } while (0)

int main(void)
{
    /* ---- core_runtime_api_version ---- */
    core_version_t v = core_runtime_api_version();
    CHECK(v.major == 1u, "api_version_major_is_1");
    CHECK(v.minor == 0u, "api_version_minor_is_0");
    CHECK(v.patch == 0u, "api_version_patch_is_0");

    /* ---- core_status_t values ---- */
    CHECK(CORE_STATUS_OK == 0,                        "status_ok_equals_zero");
    CHECK(CORE_STATUS_INVALID_ARGUMENT != CORE_STATUS_OK,   "status_invalid_arg_ne_ok");
    CHECK(CORE_STATUS_OUT_OF_MEMORY    != CORE_STATUS_OK,   "status_oom_ne_ok");
    CHECK(CORE_STATUS_NOT_FOUND        != CORE_STATUS_OK,   "status_not_found_ne_ok");
    CHECK(CORE_STATUS_UNSUPPORTED      != CORE_STATUS_OK,   "status_unsupported_ne_ok");
    CHECK(CORE_STATUS_IO_ERROR         != CORE_STATUS_OK,   "status_io_error_ne_ok");
    CHECK(CORE_STATUS_INTERNAL_ERROR   != CORE_STATUS_OK,   "status_internal_error_ne_ok");

    /* ---- core_status_succeeded / core_status_failed ---- */
    CHECK( core_status_succeeded(CORE_STATUS_OK),                "succeeded_ok");
    CHECK(!core_status_succeeded(CORE_STATUS_INVALID_ARGUMENT),  "not_succeeded_invalid");
    CHECK(!core_status_succeeded(CORE_STATUS_INTERNAL_ERROR),    "not_succeeded_internal");
    CHECK(!core_status_failed(CORE_STATUS_OK),                   "not_failed_ok");
    CHECK( core_status_failed(CORE_STATUS_INVALID_ARGUMENT),     "failed_invalid");
    CHECK( core_status_failed(CORE_STATUS_INTERNAL_ERROR),       "failed_internal");

    /* ---- core_version_make ---- */
    core_version_t v2 = core_version_make(7u, 3u, 11u);
    CHECK(v2.major == 7u,  "version_make_major");
    CHECK(v2.minor == 3u,  "version_make_minor");
    CHECK(v2.patch == 11u, "version_make_patch");

    /* ---- module_kind constants: must all be distinct ---- */
    CHECK(CORE_MODULE_KIND_UNKNOWN       != CORE_MODULE_KIND_CORE_RUNTIME,  "kind_unknown_ne_core");
    CHECK(CORE_MODULE_KIND_CORE_RUNTIME  != CORE_MODULE_KIND_LOGGING,       "kind_core_ne_logging");
    CHECK(CORE_MODULE_KIND_LOGGING       != CORE_MODULE_KIND_CONFIG,        "kind_logging_ne_config");
    CHECK(CORE_MODULE_KIND_CONFIG        != CORE_MODULE_KIND_PLATFORM,      "kind_config_ne_platform");
    CHECK(CORE_MODULE_KIND_PLATFORM      != CORE_MODULE_KIND_NET_TRANSPORT, "kind_platform_ne_net");
    CHECK(CORE_MODULE_KIND_NET_TRANSPORT != CORE_MODULE_KIND_RESOLVER,      "kind_net_ne_resolver");
    CHECK(CORE_MODULE_KIND_RESOLVER      != CORE_MODULE_KIND_SERVER_APP,    "kind_resolver_ne_server_app");

    /* ---- CORE_RUNTIME_MODULE_GET_API_V1 string constant ---- */
    CHECK(CORE_RUNTIME_MODULE_GET_API_V1[0] != '\0', "symbol_name_nonempty");

    printf("core_runtime_test: all %d checks passed\n", 28);
    return 0;
}

