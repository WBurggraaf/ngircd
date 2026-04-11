#ifndef PLATFORM_PLATFORM_H
#define PLATFORM_PLATFORM_H

#include <stdint.h>

#include "core/runtime/core_runtime.h"

#ifdef _WIN32
#  ifdef PLATFORM_BUILD
#    define PLATFORM_API __declspec(dllexport)
#  else
#    define PLATFORM_API __declspec(dllimport)
#  endif
#  define PLATFORM_CALL __cdecl
#else
#  define PLATFORM_API
#  define PLATFORM_CALL
#endif

typedef struct platform_api {
    uint32_t api_major;
    uint32_t api_minor;
    core_status_t (PLATFORM_CALL *open_file)(const char *path, int flags);
    core_status_t (PLATFORM_CALL *close_file)(int fd);
    core_status_t (PLATFORM_CALL *set_nonblock)(int fd);
    core_status_t (PLATFORM_CALL *set_cloexec)(int fd);
} platform_api_t;

typedef const platform_api_t * (PLATFORM_CALL *platform_get_api_v1_fn)(void);

PLATFORM_API const platform_api_t * PLATFORM_CALL platform_get_api_v1(void);

#endif

