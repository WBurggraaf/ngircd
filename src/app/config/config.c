#include "app/config/config.h"

#include "core/runtime/module_api.h"
#include "portab/portab.h"
#include "conn.h"
#include "conf.h"

static void CONFIG_CALL
config_fill_runtime_policy(config_bootstrap_t *bootstrap)
{
    bootstrap->ping_timeout = Conf_PingTimeout;
    bootstrap->pong_timeout = Conf_PongTimeout;
    bootstrap->connect_retry = Conf_ConnectRetry;
    bootstrap->idle_timeout = Conf_IdleTimeout;
    bootstrap->max_connections = Conf_MaxConnections;
    bootstrap->max_joins = Conf_MaxJoins;
    bootstrap->max_connections_ip = Conf_MaxConnectionsIP;
    bootstrap->max_nick_length = Conf_MaxNickLength;
    bootstrap->max_list_size = Conf_MaxListSize;
    bootstrap->max_penalty_time = Conf_MaxPenaltyTime;
    bootstrap->connect_ipv4 = Conf_ConnectIPv4;
    bootstrap->connect_ipv6 = Conf_ConnectIPv6;
    bootstrap->default_channel_modes = Conf_DefaultChannelModes;
    bootstrap->default_user_modes = Conf_DefaultUserModes;
    bootstrap->dns_enabled = Conf_DNS;
    bootstrap->ident_enabled = Conf_Ident;
    bootstrap->more_privacy = Conf_MorePrivacy;
    bootstrap->notice_before_registration = Conf_NoticeBeforeRegistration;
    bootstrap->pam_enabled = Conf_PAM;
    bootstrap->pam_is_optional = Conf_PAMIsOptional;
    bootstrap->pam_service_name = Conf_PAMServiceName;
    bootstrap->scrub_ctcp = Conf_ScrubCTCP;
}

static void CONFIG_CALL
config_fill_identity(config_bootstrap_t *bootstrap)
{
    bootstrap->api_version = 1u;
    bootstrap->server_name = Conf_ServerName;
    bootstrap->server_info = Conf_ServerInfo;
    bootstrap->server_admin1 = Conf_ServerAdmin1;
    bootstrap->server_admin2 = Conf_ServerAdmin2;
    bootstrap->server_admin_mail = Conf_ServerAdminMail;
    bootstrap->network_name = Conf_Network;
    bootstrap->listen_address = Conf_ListenAddress;
    bootstrap->oper_can_mode = Conf_OperCanMode;
    bootstrap->pid_file = Conf_PidFile;
    bootstrap->chroot_dir = Conf_Chroot;
    bootstrap->uid = Conf_UID;
    bootstrap->gid = Conf_GID;
}

static void CONFIG_CALL
config_fill_bootstrap(config_bootstrap_t *bootstrap)
{
    config_fill_identity(bootstrap);
    config_fill_runtime_policy(bootstrap);
}

static core_status_t CONFIG_CALL
config_init(void)
{
    Conf_Init();
    return CORE_STATUS_OK;
}

static core_status_t CONFIG_CALL
config_rehash(void)
{
    return Conf_Rehash() ? CORE_STATUS_OK : CORE_STATUS_INTERNAL_ERROR;
}

static int CONFIG_CALL
config_test(void)
{
    return Conf_Test();
}

static core_status_t CONFIG_CALL
config_get_bootstrap(config_bootstrap_t *bootstrap)
{
    if (bootstrap == NULL)
        return CORE_STATUS_INVALID_ARGUMENT;

    config_fill_bootstrap(bootstrap);
    return CORE_STATUS_OK;
}

static const config_api_t ConfigApi = {
    1u,
    0u,
    config_init,
    config_rehash,
    config_test,
    config_get_bootstrap
};

CONFIG_API const config_api_t * CONFIG_CALL
config_get_api_v1(void)
{
    return &ConfigApi;
}

CORE_RUNTIME_DECLARE_MODULE_API(CORE_MODULE_KIND_CONFIG, "config", &ConfigApi);

CONFIG_API const core_module_api_t * CONFIG_CALL
module_get_api_v1(void)
{
    return &module_module_api;
}

