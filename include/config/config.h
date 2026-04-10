#ifndef CONFIG_CONFIG_H
#define CONFIG_CONFIG_H

#include <stdint.h>
#include <time.h>
#include "core_runtime/core_runtime.h"

#ifdef _WIN32
#  ifdef CONFIG_BUILD
#    define CONFIG_API __declspec(dllexport)
#  else
#    define CONFIG_API __declspec(dllimport)
#  endif
#  define CONFIG_CALL __cdecl
#else
#  define CONFIG_API
#  define CONFIG_CALL
#endif

typedef struct config_bootstrap {
    uint32_t api_version;
    const char *server_name;
    const char *server_info;
    const char *server_admin1;
    const char *server_admin2;
    const char *server_admin_mail;
    const char *network_name;
    const char *listen_address;
    int oper_can_mode;
    const char *pid_file;
    const char *chroot_dir;
    uint32_t uid;
    uint32_t gid;
    int ping_timeout;
    int pong_timeout;
    int connect_retry;
    int idle_timeout;
    int max_connections;
    int max_joins;
    int max_connections_ip;
    unsigned int max_nick_length;
    int max_list_size;
    time_t max_penalty_time;
    int connect_ipv4;
    int connect_ipv6;
    const char *default_channel_modes;
    const char *default_user_modes;
    int dns_enabled;
    int ident_enabled;
    int more_privacy;
    int notice_before_registration;
    int pam_enabled;
    int pam_is_optional;
    const char *pam_service_name;
    int scrub_ctcp;
} config_bootstrap_t;

typedef struct config_api {
    uint32_t api_major;
    uint32_t api_minor;
    core_status_t (CONFIG_CALL *init)(void);
    core_status_t (CONFIG_CALL *rehash)(void);
    int (CONFIG_CALL *test)(void);
    core_status_t (CONFIG_CALL *get_bootstrap)(config_bootstrap_t *bootstrap);
} config_api_t;

typedef const config_api_t * (CONFIG_CALL *config_get_api_v1_fn)(void);

CONFIG_API const config_api_t * CONFIG_CALL config_get_api_v1(void);

#endif
