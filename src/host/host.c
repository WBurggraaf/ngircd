#include "host/host.h"

static core_status_t host_wire_modules(const host_module_set_t *modules)
{
    if (modules == NULL)
        return CORE_STATUS_INVALID_ARGUMENT;
    if (modules->logging == NULL || modules->config == NULL
        || modules->platform == NULL || modules->net_transport == NULL
        || modules->resolver == NULL)
        return CORE_STATUS_INVALID_ARGUMENT;

    if (modules->logging->api_major != 1u
        || modules->config->api_major != 1u
        || modules->platform->api_major != 1u
        || modules->net_transport->api_major != 1u
        || modules->resolver->api_major != 1u)
        return CORE_STATUS_UNSUPPORTED;

    return CORE_STATUS_OK;
}

static const host_wiring_t HostWiring = {
    1u,
    0u,
    host_wire_modules
};

const host_wiring_t *
host_get_wiring_v1(void)
{
    return &HostWiring;
}
