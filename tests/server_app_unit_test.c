/*
 * server_app_unit_test.c — unit tests for server_app module API
 *
 * Links against server_app.dll, logging.dll, config.dll, net_transport.dll.
 * Uses GetProcAddress to call module_get_api_v1 from server_app.dll
 * (avoids symbol-name collision from multiple DLLs exporting the same name).
 *
 * Tests:
 *   - server_app_get_api_v1()  struct integrity
 *   - module_get_api_v1()      metadata via GetProcAddress
 *   - create() with NULL / incomplete / valid host_module_set_t
 *   - start() / stop() lifecycle no-ops
 *   - build_proto_id() null-arg guards and happy path
 */

#include "server_app/server_app.h"   /* pulls host.h → all typed APIs */
#include "logging/logging.h"
#include "config/config.h"
#include "net_transport/net_transport.h"
#include "core_runtime/module_api.h"

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

/* ---- CHECK macro ---- */
#define CHECK(expr, name) \
    do { \
        if (!(expr)) { \
            fprintf(stderr, "FAIL: %s\n", (name)); \
            return 1; \
        } \
    } while (0)

/* Retrieve module_get_api_v1 from a loaded DLL by name */
static const core_module_api_t *
get_module_api_from(const char *dll_name)
{
#ifdef _WIN32
    HMODULE h = GetModuleHandleA(dll_name);
    if (!h) return NULL;
    FARPROC p = GetProcAddress(h, CORE_RUNTIME_MODULE_GET_API_V1);
    if (!p) return NULL;
    return ((core_module_get_api_v1_fn)(void *)p)();
#else
    (void)dll_name;
    return NULL;
#endif
}

int main(void)
{
    /* ---- server_app_get_api_v1 struct ---- */
    const server_app_api_t *api = server_app_get_api_v1();
    CHECK(api != NULL,                 "server_app_api_not_null");
    CHECK(api->api_major == 1u,        "server_app_api_major_1");
    CHECK(api->api_minor == 0u,        "server_app_api_minor_0");
    CHECK(api->create     != NULL,     "server_app_create_ptr");
    CHECK(api->start      != NULL,     "server_app_start_ptr");
    CHECK(api->stop       != NULL,     "server_app_stop_ptr");
    CHECK(api->run_loop   != NULL,     "server_app_run_loop_ptr");
    CHECK(api->build_proto_id != NULL, "server_app_build_proto_id_ptr");

    /* ---- Idempotent getter ---- */
    CHECK(server_app_get_api_v1() == api, "server_app_api_idempotent");

    /* ---- module_get_api_v1 via GetProcAddress from server_app.dll ---- */
    const core_module_api_t *m = get_module_api_from("server_app.dll");
    CHECK(m != NULL,                                             "server_app_mod_api_not_null");
    CHECK(m->api_major == 1u,                                    "server_app_mod_api_major_1");
    CHECK(m->metadata != NULL,                                   "server_app_mod_metadata_not_null");
    CHECK(m->metadata->module_kind == CORE_MODULE_KIND_SERVER_APP, "server_app_mod_kind_correct");
    CHECK(m->metadata->module_name != NULL,                      "server_app_mod_name_not_null");
    CHECK(m->metadata->module_name[0] != '\0',                   "server_app_mod_name_nonempty");
    CHECK(m->api != NULL,                                        "server_app_mod_api_ptr_not_null");

    /* ---- create() argument validation ---- */
    CHECK(api->create(NULL) == CORE_STATUS_INVALID_ARGUMENT, "create_null_modules");

    host_module_set_t ms;
    memset(&ms, 0, sizeof(ms));
    CHECK(api->create(&ms) == CORE_STATUS_INVALID_ARGUMENT, "create_all_null_fields");

    /* Only logging — config and net_transport still NULL */
    ms.logging = logging_get_api_v1();
    CHECK(api->create(&ms) == CORE_STATUS_INVALID_ARGUMENT, "create_only_logging");

    /* Add config — net_transport still NULL */
    ms.config = config_get_api_v1();
    CHECK(api->create(&ms) == CORE_STATUS_INVALID_ARGUMENT, "create_no_net_transport");

    /* Add net_transport — all three required fields present */
    ms.net_transport = net_transport_get_api_v1();
    CHECK(api->create(&ms) == CORE_STATUS_OK, "create_valid_modules");

    /* ---- start() / stop() should be no-ops returning OK ---- */
    CHECK(api->start() == CORE_STATUS_OK, "start_returns_ok");
    CHECK(api->stop()  == CORE_STATUS_OK, "stop_returns_ok");

    /* ---- build_proto_id() argument guards ---- */
    char buf[256];
    CHECK(api->build_proto_id(NULL, 0)          == CORE_STATUS_INVALID_ARGUMENT,
          "proto_id_null_buf_zero_size");
    CHECK(api->build_proto_id(NULL, sizeof(buf)) == CORE_STATUS_INVALID_ARGUMENT,
          "proto_id_null_buf_valid_size");
    CHECK(api->build_proto_id(buf,  0)           == CORE_STATUS_INVALID_ARGUMENT,
          "proto_id_valid_buf_zero_size");

    /* ---- build_proto_id() happy path (after successful create) ---- */
    memset(buf, 0, sizeof(buf));
    CHECK(api->build_proto_id(buf, sizeof(buf)) == CORE_STATUS_OK,
          "proto_id_happy_path");
    CHECK(buf[0] != '\0', "proto_id_buffer_nonempty");

    printf("server_app_unit_test: all 30 checks passed\n");
    return 0;
}
