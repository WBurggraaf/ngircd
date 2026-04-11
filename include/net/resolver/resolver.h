#ifndef RESOLVER_RESOLVER_H
#define RESOLVER_RESOLVER_H

#include <stdint.h>

#include "core/runtime/core_runtime.h"

#ifdef _WIN32
#  ifdef RESOLVER_BUILD
#    define RESOLVER_API __declspec(dllexport)
#  else
#    define RESOLVER_API __declspec(dllimport)
#  endif
#  define RESOLVER_CALL __cdecl
#else
#  define RESOLVER_API
#  define RESOLVER_CALL
#endif

typedef struct resolver_api {
    uint32_t api_major;
    uint32_t api_minor;
    core_status_t (RESOLVER_CALL *resolve_name)(const char *host);
    core_status_t (RESOLVER_CALL *resolve_addr_ident)(const void *addr, uint32_t addr_size);
} resolver_api_t;

typedef const resolver_api_t * (RESOLVER_CALL *resolver_get_api_v1_fn)(void);

RESOLVER_API const resolver_api_t * RESOLVER_CALL resolver_get_api_v1(void);

#endif

