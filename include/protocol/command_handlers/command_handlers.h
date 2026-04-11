#ifndef COMMAND_HANDLERS_COMMAND_HANDLERS_H
#define COMMAND_HANDLERS_COMMAND_HANDLERS_H

#include <stdint.h>

#include "core/runtime/core_runtime.h"

#ifdef _WIN32
#  ifdef COMMAND_HANDLERS_BUILD
#    define COMMAND_HANDLERS_API __declspec(dllexport)
#  else
#    define COMMAND_HANDLERS_API __declspec(dllimport)
#  endif
#  define COMMAND_HANDLERS_CALL __cdecl
#else
#  define COMMAND_HANDLERS_API
#  define COMMAND_HANDLERS_CALL
#endif

typedef struct command_handlers_api {
    uint32_t api_major;
    uint32_t api_minor;
    core_status_t (COMMAND_HANDLERS_CALL *init)(void);
    core_status_t (COMMAND_HANDLERS_CALL *shutdown)(void);
    core_status_t (COMMAND_HANDLERS_CALL *dispatch)(const char *command_name);
} command_handlers_api_t;

typedef const command_handlers_api_t * (COMMAND_HANDLERS_CALL *command_handlers_get_api_v1_fn)(void);

COMMAND_HANDLERS_API const command_handlers_api_t * COMMAND_HANDLERS_CALL command_handlers_get_api_v1(void);

#endif

