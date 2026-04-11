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
#include "protocol/irc_protocol/irc_protocol.h"
#include "protocol/command_handlers/command_handlers.h"
#include "state/client_state/client_state.h"
#include "state/channel_state/channel_state.h"
#include "app/server_app/server_app.h"

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
    CHECK(w->wire(&ms) == CORE_STATUS_INVALID_ARGUMENT, "wire_no_irc_protocol");

    MOCK_API(irc_protocol_api_t, mock_irc, 1u);
    ms.irc_protocol = &mock_irc;
    CHECK(w->wire(&ms) == CORE_STATUS_INVALID_ARGUMENT, "wire_no_client_state");

    MOCK_API(client_state_api_t, mock_cst, 1u);
    ms.client_state = &mock_cst;
    CHECK(w->wire(&ms) == CORE_STATUS_INVALID_ARGUMENT, "wire_no_channel_state");

    MOCK_API(channel_state_api_t, mock_chan, 1u);
    ms.channel_state = &mock_chan;
    CHECK(w->wire(&ms) == CORE_STATUS_INVALID_ARGUMENT, "wire_no_command_handlers");

    MOCK_API(command_handlers_api_t, mock_cmd, 1u);
    ms.command_handlers = &mock_cmd;
    CHECK(w->wire(&ms) == CORE_STATUS_INVALID_ARGUMENT, "wire_no_server_app");

    MOCK_API(server_app_api_t, mock_sapp, 1u);
    ms.server_app = &mock_sapp;

    /* ---- All 10 fields present with api_major == 1 → OK ---- */
    CHECK(w->wire(&ms) == CORE_STATUS_OK, "wire_all_valid");

    /* ---- Wrong api_major in each slot → UNSUPPORTED ---- */
    MOCK_API(logging_api_t, bad_log, 2u);
    ms.logging = &bad_log;
    CHECK(w->wire(&ms) == CORE_STATUS_UNSUPPORTED, "wire_bad_logging_version");
    ms.logging = &mock_log;

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

    MOCK_API(irc_protocol_api_t, bad_irc, 2u);
    ms.irc_protocol = &bad_irc;
    CHECK(w->wire(&ms) == CORE_STATUS_UNSUPPORTED, "wire_bad_irc_protocol_version");
    ms.irc_protocol = &mock_irc;

    MOCK_API(client_state_api_t, bad_cst, 2u);
    ms.client_state = &bad_cst;
    CHECK(w->wire(&ms) == CORE_STATUS_UNSUPPORTED, "wire_bad_client_state_version");
    ms.client_state = &mock_cst;

    MOCK_API(channel_state_api_t, bad_chan, 2u);
    ms.channel_state = &bad_chan;
    CHECK(w->wire(&ms) == CORE_STATUS_UNSUPPORTED, "wire_bad_channel_state_version");
    ms.channel_state = &mock_chan;

    MOCK_API(command_handlers_api_t, bad_cmd, 2u);
    ms.command_handlers = &bad_cmd;
    CHECK(w->wire(&ms) == CORE_STATUS_UNSUPPORTED, "wire_bad_command_handlers_version");
    ms.command_handlers = &mock_cmd;

    MOCK_API(server_app_api_t, bad_sapp, 2u);
    ms.server_app = &bad_sapp;
    CHECK(w->wire(&ms) == CORE_STATUS_UNSUPPORTED, "wire_bad_server_app_version");
    ms.server_app = &mock_sapp;

    /* ---- Back to all valid after all restores ---- */
    CHECK(w->wire(&ms) == CORE_STATUS_OK, "wire_all_valid_after_restores");

    /* ---- api_major == 0 (edge: exactly zero) ---- */
    MOCK_API(logging_api_t, zero_log, 0u);
    ms.logging = &zero_log;
    CHECK(w->wire(&ms) == CORE_STATUS_UNSUPPORTED, "wire_logging_major_zero");
    ms.logging = &mock_log;

    printf("host_wiring_test: all 34 checks passed\n");
    return 0;
}

