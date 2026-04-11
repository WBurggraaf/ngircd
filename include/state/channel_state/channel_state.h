#ifndef CHANNEL_STATE_CHANNEL_STATE_H
#define CHANNEL_STATE_CHANNEL_STATE_H

#include <stdint.h>

#include "core/runtime/core_runtime.h"

#ifdef _WIN32
#  ifdef CHANNEL_STATE_BUILD
#    define CHANNEL_STATE_API __declspec(dllexport)
#  else
#    define CHANNEL_STATE_API __declspec(dllimport)
#  endif
#  define CHANNEL_STATE_CALL __cdecl
#else
#  define CHANNEL_STATE_API
#  define CHANNEL_STATE_CALL
#endif

typedef uint32_t channel_state_id_t;

typedef struct channel_state_api {
    uint32_t api_major;
    uint32_t api_minor;
    core_status_t (CHANNEL_STATE_CALL *init)(void);
    core_status_t (CHANNEL_STATE_CALL *shutdown)(void);
    core_status_t (CHANNEL_STATE_CALL *create_channel)(channel_state_id_t channel_id);
} channel_state_api_t;

typedef const channel_state_api_t * (CHANNEL_STATE_CALL *channel_state_get_api_v1_fn)(void);

CHANNEL_STATE_API const channel_state_api_t * CHANNEL_STATE_CALL channel_state_get_api_v1(void);

#endif

