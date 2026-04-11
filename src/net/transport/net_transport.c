#include "net/transport/net_transport.h"

#include "core/runtime/module_api.h"
#include "portab/portab.h"
#include "conn.h"
#include "ngircd/io.h"
#include "win32compat.h"

#include <sys/time.h>

static core_status_t NET_TRANSPORT_CALL
net_transport_init(uint32_t connection_capacity)
{
    return io_library_init((unsigned int)connection_capacity) ? CORE_STATUS_OK : CORE_STATUS_INTERNAL_ERROR;
}

static core_status_t NET_TRANSPORT_CALL
net_transport_init_connections(void)
{
    Conn_Init();
    return CORE_STATUS_OK;
}

static core_status_t NET_TRANSPORT_CALL
net_transport_init_listeners(void)
{
    return Conn_InitListeners() > 0 ? CORE_STATUS_OK : CORE_STATUS_INTERNAL_ERROR;
}

static core_status_t NET_TRANSPORT_CALL
net_transport_shutdown_listeners(void)
{
    Conn_ExitListeners();
    return CORE_STATUS_OK;
}

static core_status_t NET_TRANSPORT_CALL
net_transport_run(void)
{
    Conn_Handler();
    return CORE_STATUS_OK;
}

static core_status_t NET_TRANSPORT_CALL
net_transport_close_all_sockets(int except_of)
{
    Conn_CloseAllSockets(except_of);
    return CORE_STATUS_OK;
}

static core_status_t NET_TRANSPORT_CALL
net_transport_start_login(uint32_t connection_id)
{
    Conn_StartLogin((CONN_ID)connection_id);
    return CORE_STATUS_OK;
}

static core_status_t NET_TRANSPORT_CALL
net_transport_close_connection(uint32_t connection_id, const char *log_msg, const char *fwd_msg, int inform_client)
{
    Conn_Close((CONN_ID)connection_id, log_msg, fwd_msg, inform_client != 0);
    return CORE_STATUS_OK;
}

static const char * NET_TRANSPORT_CALL
net_transport_get_password(uint32_t connection_id)
{
    return Conn_Password((CONN_ID)connection_id);
}

static core_status_t NET_TRANSPORT_CALL
net_transport_set_password(uint32_t connection_id, const char *password)
{
    Conn_SetPassword((CONN_ID)connection_id, password);
    return CORE_STATUS_OK;
}

static uint32_t NET_TRANSPORT_CALL
net_transport_get_from_proc(int fd)
{
    return (uint32_t)Conn_GetFromProc(fd);
}

static void * NET_TRANSPORT_CALL
net_transport_get_client(uint32_t connection_id)
{
    return Conn_GetClient((CONN_ID)connection_id);
}

static void * NET_TRANSPORT_CALL
net_transport_get_proc_stat(uint32_t connection_id)
{
    return Conn_GetProcStat((CONN_ID)connection_id);
}

static int NET_TRANSPORT_CALL
net_transport_uses_ssl(uint32_t connection_id)
{
    return Conn_UsesSSL((CONN_ID)connection_id) ? 1 : 0;
}

static const char * NET_TRANSPORT_CALL
net_transport_get_cert_fp(uint32_t connection_id)
{
    return Conn_GetCertFp((CONN_ID)connection_id);
}

static const char * NET_TRANSPORT_CALL
net_transport_get_ipa_info(uint32_t connection_id)
{
    return Conn_GetIPAInfo((CONN_ID)connection_id);
}

static core_status_t NET_TRANSPORT_CALL
net_transport_set_cert_fp(uint32_t connection_id, const char *fingerprint)
{
    return Conn_SetCertFp((CONN_ID)connection_id, fingerprint) ? CORE_STATUS_OK : CORE_STATUS_INTERNAL_ERROR;
}

static core_status_t NET_TRANSPORT_CALL
net_transport_get_cipher_info(uint32_t connection_id, char *buffer, uint32_t buffer_len)
{
#ifdef SSL_SUPPORT
    return Conn_GetCipherInfo((CONN_ID)connection_id, buffer, (size_t)buffer_len) ? CORE_STATUS_OK : CORE_STATUS_INTERNAL_ERROR;
#else
    (void)connection_id;
    (void)buffer;
    (void)buffer_len;
    return CORE_STATUS_INTERNAL_ERROR;
#endif
}

static int NET_TRANSPORT_CALL
net_transport_get_auth_ping(uint32_t connection_id)
{
#ifndef STRICT_RFC
    return (int)Conn_GetAuthPing((CONN_ID)connection_id);
#else
    (void)connection_id;
    return 0;
#endif
}

static void NET_TRANSPORT_CALL
net_transport_set_auth_ping(uint32_t connection_id, int id)
{
#ifndef STRICT_RFC
    Conn_SetAuthPing((CONN_ID)connection_id, (long)id);
#else
    (void)connection_id;
    (void)id;
#endif
}

static uint32_t NET_TRANSPORT_CALL
net_transport_count(void)
{
    return (uint32_t)Conn_Count();
}

static uint32_t NET_TRANSPORT_CALL
net_transport_count_max(void)
{
    return (uint32_t)Conn_CountMax();
}

static uint32_t NET_TRANSPORT_CALL
net_transport_count_accepted(void)
{
    return (uint32_t)Conn_CountAccepted();
}

static core_status_t NET_TRANSPORT_CALL
net_transport_shutdown_connections(void)
{
    Conn_Exit();
    return CORE_STATUS_OK;
}

static core_status_t NET_TRANSPORT_CALL
net_transport_shutdown(void)
{
    io_library_shutdown();
    return CORE_STATUS_OK;
}

static core_status_t NET_TRANSPORT_CALL
net_transport_dispatch(uint32_t timeout_ms)
{
    struct timeval tv;

    tv.tv_sec = (long)(timeout_ms / 1000u);
    tv.tv_usec = (long)((timeout_ms % 1000u) * 1000u);
    return io_dispatch(&tv) >= 0 ? CORE_STATUS_OK : CORE_STATUS_INTERNAL_ERROR;
}

static const net_transport_api_t NetTransportApi = {
    1u,
    0u,
    net_transport_init,
    net_transport_init_connections,
    net_transport_init_listeners,
    net_transport_shutdown_listeners,
    net_transport_run,
    net_transport_close_all_sockets,
    net_transport_start_login,
    net_transport_close_connection,
    net_transport_get_password,
    net_transport_set_password,
    net_transport_get_from_proc,
    net_transport_get_client,
    net_transport_get_proc_stat,
    net_transport_uses_ssl,
    net_transport_get_cert_fp,
    net_transport_get_ipa_info,
    net_transport_set_cert_fp,
    net_transport_get_cipher_info,
    net_transport_get_auth_ping,
    net_transport_set_auth_ping,
    net_transport_count,
    net_transport_count_max,
    net_transport_count_accepted,
    net_transport_shutdown_connections,
    net_transport_shutdown,
    net_transport_dispatch
};

NET_TRANSPORT_API const net_transport_api_t * NET_TRANSPORT_CALL
net_transport_get_api_v1(void)
{
    return &NetTransportApi;
}

static const core_module_metadata_t NetTransportMetadata = {
    1u, 0u, CORE_MODULE_KIND_NET_TRANSPORT, "net_transport", "1.0"
};

static const core_module_api_t NetTransportModuleApi = {
    1u, 0u, &NetTransportMetadata, &NetTransportApi
};

NET_TRANSPORT_API const core_module_api_t * NET_TRANSPORT_CALL
module_get_api_v1(void)
{
    return &NetTransportModuleApi;
}

