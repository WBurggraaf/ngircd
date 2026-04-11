#include "protocol/command_handlers/command_handlers.h"

static const command_handlers_api_t CommandHandlersApi = {
    1u,
    0u,
    0,
    0,
    0
};

COMMAND_HANDLERS_API const command_handlers_api_t * COMMAND_HANDLERS_CALL
command_handlers_get_api_v1(void)
{
    return &CommandHandlersApi;
}

