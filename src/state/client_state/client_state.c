#include "state/client_state/client_state.h"
#include "core/runtime/module_api.h"

static const client_state_api_t ClientStateApi = {
    1u,
    0u,
    0,
    0,
    0
};

CLIENT_STATE_API const client_state_api_t * CLIENT_STATE_CALL
client_state_get_api_v1(void)
{
    return &ClientStateApi;
}

CORE_RUNTIME_DECLARE_MODULE_API(CORE_MODULE_KIND_CLIENT_STATE, "client_state", &ClientStateApi);

CLIENT_STATE_API const core_module_api_t * CLIENT_STATE_CALL
module_get_api_v1(void)
{
    return &module_module_api;
}

