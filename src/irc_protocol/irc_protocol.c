#include "irc_protocol/irc_protocol.h"

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
