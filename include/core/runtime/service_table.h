#ifndef CORE_RUNTIME_SERVICE_TABLE_H
#define CORE_RUNTIME_SERVICE_TABLE_H

#include "core_runtime.h"

typedef struct core_service_table {
    uint32_t api_major;
    uint32_t api_minor;
    const core_allocator_t *allocator;
} core_service_table_t;

#endif
