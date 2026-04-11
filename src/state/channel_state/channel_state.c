#include "state/channel_state/channel_state.h"
#include "core/runtime/module_api.h"

static const channel_state_api_t ChannelStateApi = {
    1u,
    0u,
    0,
    0,
    0
};

CHANNEL_STATE_API const channel_state_api_t * CHANNEL_STATE_CALL
channel_state_get_api_v1(void)
{
    return &ChannelStateApi;
}

CORE_RUNTIME_DECLARE_MODULE_API(CORE_MODULE_KIND_CHANNEL_STATE, "channel_state", &ChannelStateApi);

CHANNEL_STATE_API const core_module_api_t * CHANNEL_STATE_CALL
module_get_api_v1(void)
{
    return &module_module_api;
}

