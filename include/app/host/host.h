#ifndef HOST_HOST_H
#define HOST_HOST_H

#include <stdint.h>

#include "core/runtime/core_runtime.h"
#include "core/runtime/module_api.h"
#include "app/config/config.h"
#include "app/logging/logging.h"
#include "platform/platform.h"
#include "net/transport/net_transport.h"
#include "net/resolver/resolver.h"

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

#ifdef _WIN32
#  ifdef HOST_BUILD
#    define HOST_API __declspec(dllexport)
#  else
#    define HOST_API __declspec(dllimport)
#  endif
#  define HOST_CALL __cdecl
#else
#  define HOST_API
#  define HOST_CALL
#endif

/*
 * host_load_modules: dynamically loads first-wave DLLs using the OS loader,
 * resolves module_get_api_v1 from each, validates API major versions, and
 * fills *out with typed API pointers.
 *
 * On Windows, DLLs are searched with LoadLibraryA using the standard DLL
 * search order (EXE directory, PATH, etc.).  Pass dll_dir = NULL to use the
 * default search order, or a directory path that is prepended to each name.
 *
 * Returns CORE_STATUS_OK on success.  On failure, *out is zeroed and an
 * error description is written to errbuf (if non-NULL, up to errbuf_size bytes).
 */
HOST_API core_status_t HOST_CALL
host_load_modules(const char *dll_dir, host_module_set_t *out,
                  char *errbuf, size_t errbuf_size);

const host_wiring_t *host_get_wiring_v1(void);

#endif

