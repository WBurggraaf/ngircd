#include "protocol/irc_protocol/irc_protocol.h"
#include "core/runtime/module_api.h"

static const irc_protocol_api_t IrcProtocolApi = {
    1u,
    0u,
    0,
    0
};

IRC_PROTOCOL_API const irc_protocol_api_t * IRC_PROTOCOL_CALL
irc_protocol_get_api_v1(void)
{
    return &IrcProtocolApi;
}

CORE_RUNTIME_DECLARE_MODULE_API(CORE_MODULE_KIND_IRC_PROTOCOL, "irc_protocol", &IrcProtocolApi);

IRC_PROTOCOL_API const core_module_api_t * IRC_PROTOCOL_CALL
module_get_api_v1(void)
{
    return &module_module_api;
}

