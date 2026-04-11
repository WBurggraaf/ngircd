#ifndef NET_TRANSPORT_NET_TRANSPORT_H
#define NET_TRANSPORT_NET_TRANSPORT_H

#include <stdint.h>

#include "core/runtime/core_runtime.h"

#ifdef _WIN32
#  ifdef NET_TRANSPORT_BUILD
#    define NET_TRANSPORT_API __declspec(dllexport)
#  else
#    define NET_TRANSPORT_API __declspec(dllimport)
#  endif
#  define NET_TRANSPORT_CALL __cdecl
#else
#  define NET_TRANSPORT_API
#  define NET_TRANSPORT_CALL
#endif

typedef uint32_t net_transport_conn_id_t;

typedef struct net_transport_api {
    uint32_t api_major;
    uint32_t api_minor;
    core_status_t (NET_TRANSPORT_CALL *init)(uint32_t connection_capacity);
    core_status_t (NET_TRANSPORT_CALL *init_connections)(void);
    core_status_t (NET_TRANSPORT_CALL *init_listeners)(void);
    core_status_t (NET_TRANSPORT_CALL *shutdown_listeners)(void);
    core_status_t (NET_TRANSPORT_CALL *run)(void);
    core_status_t (NET_TRANSPORT_CALL *close_all_sockets)(int except_of);
    core_status_t (NET_TRANSPORT_CALL *start_login)(uint32_t connection_id);
    core_status_t (NET_TRANSPORT_CALL *close_connection)(uint32_t connection_id, const char *log_msg, const char *fwd_msg, int inform_client);
    const char * (NET_TRANSPORT_CALL *get_password)(uint32_t connection_id);
    core_status_t (NET_TRANSPORT_CALL *set_password)(uint32_t connection_id, const char *password);
    uint32_t (NET_TRANSPORT_CALL *get_from_proc)(int fd);
    void * (NET_TRANSPORT_CALL *get_client)(uint32_t connection_id);
    void * (NET_TRANSPORT_CALL *get_proc_stat)(uint32_t connection_id);
    int (NET_TRANSPORT_CALL *uses_ssl)(uint32_t connection_id);
    const char * (NET_TRANSPORT_CALL *get_cert_fp)(uint32_t connection_id);
    const char * (NET_TRANSPORT_CALL *get_ipa_info)(uint32_t connection_id);
    core_status_t (NET_TRANSPORT_CALL *set_cert_fp)(uint32_t connection_id, const char *fingerprint);
    core_status_t (NET_TRANSPORT_CALL *get_cipher_info)(uint32_t connection_id, char *buffer, uint32_t buffer_len);
    int (NET_TRANSPORT_CALL *get_auth_ping)(uint32_t connection_id);
    void (NET_TRANSPORT_CALL *set_auth_ping)(uint32_t connection_id, int id);
    uint32_t (NET_TRANSPORT_CALL *count)(void);
    uint32_t (NET_TRANSPORT_CALL *count_max)(void);
    uint32_t (NET_TRANSPORT_CALL *count_accepted)(void);
    core_status_t (NET_TRANSPORT_CALL *shutdown_connections)(void);
    core_status_t (NET_TRANSPORT_CALL *shutdown)(void);
    core_status_t (NET_TRANSPORT_CALL *dispatch)(uint32_t timeout_ms);
} net_transport_api_t;

typedef const net_transport_api_t * (NET_TRANSPORT_CALL *net_transport_get_api_v1_fn)(void);

NET_TRANSPORT_API const net_transport_api_t * NET_TRANSPORT_CALL net_transport_get_api_v1(void);

#endif

