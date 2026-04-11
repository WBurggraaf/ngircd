#ifndef CORE_RUNTIME_MODULE_API_H
#define CORE_RUNTIME_MODULE_API_H

#include "core_runtime.h"

#define CORE_RUNTIME_MODULE_GET_API_V1 "module_get_api_v1"

/* Values for core_module_metadata_t::module_kind */
#define CORE_MODULE_KIND_UNKNOWN        0u
#define CORE_MODULE_KIND_CORE_RUNTIME   1u
#define CORE_MODULE_KIND_LOGGING        2u
#define CORE_MODULE_KIND_CONFIG         3u
#define CORE_MODULE_KIND_PLATFORM       4u
#define CORE_MODULE_KIND_NET_TRANSPORT  5u
#define CORE_MODULE_KIND_RESOLVER       6u
#define CORE_MODULE_KIND_SERVER_APP     7u

typedef struct core_module_api {
    uint32_t api_major;
    uint32_t api_minor;
    const core_module_metadata_t *metadata;
    /* Pointer to the module-specific typed API struct.
     * Cast based on metadata->module_kind. */
    const void *api;
} core_module_api_t;

typedef const core_module_api_t * (CORE_RUNTIME_CALL *core_module_get_api_v1_fn)(void);

#endif
