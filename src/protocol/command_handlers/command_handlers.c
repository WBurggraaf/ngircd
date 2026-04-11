#include "protocol/command_handlers/command_handlers.h"
#include "core/runtime/module_api.h"

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

CORE_RUNTIME_DECLARE_MODULE_API(CORE_MODULE_KIND_COMMAND_HANDLERS, "command_handlers", &CommandHandlersApi);

COMMAND_HANDLERS_API const core_module_api_t * COMMAND_HANDLERS_CALL
module_get_api_v1(void)
{
    return &module_module_api;
}

