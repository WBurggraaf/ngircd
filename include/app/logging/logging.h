#ifndef LOGGING_LOGGING_H
#define LOGGING_LOGGING_H

#include <stdarg.h>
#include <stdint.h>

#include "core/runtime/core_runtime.h"

#ifdef _WIN32
#  ifdef LOGGING_BUILD
#    define LOGGING_API __declspec(dllexport)
#  else
#    define LOGGING_API __declspec(dllimport)
#  endif
#  define LOGGING_CALL __cdecl
#else
#  define LOGGING_API
#  define LOGGING_CALL
#endif

typedef struct logging_api {
    uint32_t api_major;
    uint32_t api_minor;
    core_status_t (LOGGING_CALL *init)(int syslog_mode);
    void (LOGGING_CALL *shutdown)(void);
    void (LOGGING_CALL *reinit)(void);
    void (LOGGING_CALL *log)(int level, const char *format, ...);
    void (LOGGING_CALL *debug)(const char *format, ...);
} logging_api_t;

typedef const logging_api_t * (LOGGING_CALL *logging_get_api_v1_fn)(void);

LOGGING_API const logging_api_t * LOGGING_CALL logging_get_api_v1(void);

#endif

