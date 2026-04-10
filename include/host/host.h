#ifndef HOST_HOST_H
#define HOST_HOST_H

#include <stdint.h>

#include "core_runtime/core_runtime.h"
#include "core_runtime/module_api.h"
#include "config/config.h"
#include "logging/logging.h"
#include "platform/platform.h"
#include "net_transport/net_transport.h"
#include "resolver/resolver.h"

typedef struct host_module_set {
    const core_module_api_t *core_runtime;
    const logging_api_t *logging;
    const config_api_t *config;
    const platform_api_t *platform;
    const net_transport_api_t *net_transport;
    const resolver_api_t *resolver;
} host_module_set_t;

typedef struct host_wiring {
    uint32_t api_major;
    uint32_t api_minor;
    core_status_t (*wire)(const host_module_set_t *modules);
} host_wiring_t;

const host_wiring_t *host_get_wiring_v1(void);

#endif
