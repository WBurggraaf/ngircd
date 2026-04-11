/*
 * host_wiring_test.c — unit tests for host wiring validation logic
 *
 * Tests host_get_wiring_v1() and the wire() function with:
 *   - NULL module set
 *   - Incrementally filled module sets (missing fields)
 *   - All fields present but with wrong api_major values
 *   - Fully valid module set
 *
 * Compiles host.c directly with HOST_BUILD.  Mock API structs are built
 * on the stack using memset; no DLL deps required.
 */

#include "app/host/host.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr, name) \
    do { \
        if (!(expr)) { \
            fprintf(stderr, "FAIL: %s\n", (name)); \
            return 1; \
        } \
    } while (0)

/* Build a zeroed mock struct and set api_major */
#define MOCK_API(type, varname, major) \
    type varname; \
    memset(&varname, 0, sizeof(varname)); \
    varname.api_major = (major)

int main(void)
{
    /* ---- host_get_wiring_v1 basics ---- */
    const host_wiring_t *w = host_get_wiring_v1();
    CHECK(w != NULL,       "wiring_not_null");
    CHECK(w->api_major == 1u, "wiring_api_major_1");
    CHECK(w->api_minor == 0u, "wiring_api_minor_0");
    CHECK(w->wire != NULL, "wiring_fn_not_null");

    /* ---- NULL module set ---- */
    CHECK(w->wire(NULL) == CORE_STATUS_INVALID_ARGUMENT, "wire_null_set");

    /* ---- Build up module set incrementally, expect INVALID_ARGUMENT each step ---- */
    host_module_set_t ms;
    memset(&ms, 0, sizeof(ms));

    CHECK(w->wire(&ms) == CORE_STATUS_INVALID_ARGUMENT, "wire_all_null_fields");

    MOCK_API(logging_api_t, mock_log, 1u);
    ms.logging = &mock_log;
    CHECK(w->wire(&ms) == CORE_STATUS_INVALID_ARGUMENT, "wire_only_logging");

    MOCK_API(config_api_t, mock_cfg, 1u);
    ms.config = &mock_cfg;
    CHECK(w->wire(&ms) == CORE_STATUS_INVALID_ARGUMENT, "wire_logging_plus_config");

    MOCK_API(platform_api_t, mock_plat, 1u);
    ms.platform = &mock_plat;
    CHECK(w->wire(&ms) == CORE_STATUS_INVALID_ARGUMENT, "wire_no_net_transport");

    MOCK_API(net_transport_api_t, mock_net, 1u);
    ms.net_transport = &mock_net;
    CHECK(w->wire(&ms) == CORE_STATUS_INVALID_ARGUMENT, "wire_no_resolver");

    MOCK_API(resolver_api_t, mock_res, 1u);
    ms.resolver = &mock_res;

    /* ---- All fields present with api_major == 1 → OK ---- */
    CHECK(w->wire(&ms) == CORE_STATUS_OK, "wire_all_valid");

    /* ---- Wrong api_major in each slot → UNSUPPORTED ---- */
    MOCK_API(logging_api_t, bad_log, 2u);
    ms.logging = &bad_log;
    CHECK(w->wire(&ms) == CORE_STATUS_UNSUPPORTED, "wire_bad_logging_version");
    ms.logging = &mock_log;                     /* restore */

    MOCK_API(config_api_t, bad_cfg, 0u);
    ms.config = &bad_cfg;
    CHECK(w->wire(&ms) == CORE_STATUS_UNSUPPORTED, "wire_bad_config_version");
    ms.config = &mock_cfg;

    MOCK_API(platform_api_t, bad_plat, 99u);
    ms.platform = &bad_plat;
    CHECK(w->wire(&ms) == CORE_STATUS_UNSUPPORTED, "wire_bad_platform_version");
    ms.platform = &mock_plat;

    MOCK_API(net_transport_api_t, bad_net, 2u);
    ms.net_transport = &bad_net;
    CHECK(w->wire(&ms) == CORE_STATUS_UNSUPPORTED, "wire_bad_net_transport_version");
    ms.net_transport = &mock_net;

    MOCK_API(resolver_api_t, bad_res, 2u);
    ms.resolver = &bad_res;
    CHECK(w->wire(&ms) == CORE_STATUS_UNSUPPORTED, "wire_bad_resolver_version");
    ms.resolver = &mock_res;

    /* ---- Back to all valid after all restores ---- */
    CHECK(w->wire(&ms) == CORE_STATUS_OK, "wire_all_valid_after_restores");

    /* ---- api_major == 0 (edge: exactly zero) ---- */
    MOCK_API(logging_api_t, zero_log, 0u);
    ms.logging = &zero_log;
    CHECK(w->wire(&ms) == CORE_STATUS_UNSUPPORTED, "wire_logging_major_zero");
    ms.logging = &mock_log;

    printf("host_wiring_test: all 19 checks passed\n");
    return 0;
}

