#include "state/channel_state/channel_state.h"

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

