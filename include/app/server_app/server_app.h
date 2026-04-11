#ifndef SERVER_APP_SERVER_APP_H
#define SERVER_APP_SERVER_APP_H

#include <stdint.h>
#include <stddef.h>

#include "core/runtime/core_runtime.h"
#include "app/host/host.h"

#ifdef _WIN32
#  ifdef SERVER_APP_BUILD
#    define SERVER_APP_API __declspec(dllexport)
#  else
#    define SERVER_APP_API __declspec(dllimport)
#  endif
#  define SERVER_APP_CALL __cdecl
#else
#  define SERVER_APP_API
#  define SERVER_APP_CALL
#endif

typedef struct server_app_api {
    uint32_t api_major;
    uint32_t api_minor;
    core_status_t (SERVER_APP_CALL *create)(const host_module_set_t *modules);
    core_status_t (SERVER_APP_CALL *start)(void);
    core_status_t (SERVER_APP_CALL *stop)(void);
    core_status_t (SERVER_APP_CALL *init_runtime)(void);
    core_status_t (SERVER_APP_CALL *init_random)(void);
    core_status_t (SERVER_APP_CALL *init_preloop)(int syslog_mode);
    core_status_t (SERVER_APP_CALL *init_runtime_loop)(void);
    core_status_t (SERVER_APP_CALL *build_proto_id)(char *buffer, size_t buffer_size);
    core_status_t (SERVER_APP_CALL *init_predefined_channels)(void);
    core_status_t (SERVER_APP_CALL *init_listeners)(void);
    core_status_t (SERVER_APP_CALL *create_pidfile)(void);
    core_status_t (SERVER_APP_CALL *run_loop)(void);
    core_status_t (SERVER_APP_CALL *shutdown_runtime)(void);
    core_status_t (SERVER_APP_CALL *shutdown_runtime_loop)(void);
    core_status_t (SERVER_APP_CALL *delete_pidfile)(void);
    core_status_t (SERVER_APP_CALL *init_daemon)(int nodaemon);
} server_app_api_t;

typedef const server_app_api_t * (SERVER_APP_CALL *server_app_get_api_v1_fn)(void);

SERVER_APP_API const server_app_api_t * SERVER_APP_CALL server_app_get_api_v1(void);

#endif

