#ifndef CLIENT_STATE_CLIENT_STATE_H
#define CLIENT_STATE_CLIENT_STATE_H

#include <stdint.h>

#include "core/runtime/core_runtime.h"

#ifdef _WIN32
#  ifdef CLIENT_STATE_BUILD
#    define CLIENT_STATE_API __declspec(dllexport)
#  else
#    define CLIENT_STATE_API __declspec(dllimport)
#  endif
#  define CLIENT_STATE_CALL __cdecl
#else
#  define CLIENT_STATE_API
#  define CLIENT_STATE_CALL
#endif

typedef uint32_t client_state_id_t;

typedef struct client_state_api {
    uint32_t api_major;
    uint32_t api_minor;
    core_status_t (CLIENT_STATE_CALL *init)(void);
    core_status_t (CLIENT_STATE_CALL *shutdown)(void);
    core_status_t (CLIENT_STATE_CALL *register_client)(client_state_id_t client_id);
} client_state_api_t;

typedef const client_state_api_t * (CLIENT_STATE_CALL *client_state_get_api_v1_fn)(void);

CLIENT_STATE_API const client_state_api_t * CLIENT_STATE_CALL client_state_get_api_v1(void);

#endif

