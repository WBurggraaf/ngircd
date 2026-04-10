#ifndef CORE_RUNTIME_MODULE_API_H
#define CORE_RUNTIME_MODULE_API_H

#include "core_runtime.h"

#define CORE_RUNTIME_MODULE_GET_API_V1 "module_get_api_v1"

typedef struct core_module_api {
    uint32_t api_major;
    uint32_t api_minor;
    const core_module_metadata_t *metadata;
} core_module_api_t;

typedef const core_module_api_t * (CORE_RUNTIME_CALL *core_module_get_api_v1_fn)(void);

#endif
