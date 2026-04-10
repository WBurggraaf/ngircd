#include "client_state/client_state.h"

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
