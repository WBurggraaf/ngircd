#include "app/server_app/server_app.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <syslog.h>

#ifdef _WIN32
#include "config.h"
#include "win32compat.h"
#include <sys/stat.h>
#else
#include <unistd.h>
#endif

#include "core/runtime/module_api.h"
#include "portab/portab.h"
#include "defines.h"
#include "ngircd.h"

/* Cached module APIs populated by server_app_create() from the injected table */
static const config_api_t        *s_config_api    = NULL;
static const logging_api_t       *s_logging_api   = NULL;
static const net_transport_api_t *s_transport_api = NULL;

static core_status_t SERVER_APP_CALL
server_app_create(const host_module_set_t *modules)
{
    if (!modules)
        return CORE_STATUS_INVALID_ARGUMENT;
    if (!modules->config || !modules->logging || !modules->net_transport)
        return CORE_STATUS_INVALID_ARGUMENT;
    s_config_api    = modules->config;
    s_logging_api   = modules->logging;
    s_transport_api = modules->net_transport;
    return CORE_STATUS_OK;
}

static core_status_t SERVER_APP_CALL
server_app_start(void)
{
    return CORE_STATUS_OK;
}

static core_status_t SERVER_APP_CALL
server_app_stop(void)
{
    return CORE_STATUS_OK;
}

static core_status_t
server_app_get_bootstrap(config_bootstrap_t *bootstrap)
{
    if (!s_config_api || !s_config_api->get_bootstrap || !bootstrap)
        return CORE_STATUS_INTERNAL_ERROR;
    return s_config_api->get_bootstrap(bootstrap);
}

static core_status_t SERVER_APP_CALL
server_app_init_runtime(void)
{
    ngircd_set_start_time();
    return CORE_STATUS_OK;
}

static core_status_t
server_app_random_init_kern(const char *file)
{
    unsigned int seed;
    bool ret = false;
    int fd = open(file, O_RDONLY);

    if (fd >= 0) {
        if (read(fd, &seed, sizeof(seed)) == sizeof(seed))
            ret = true;
        close(fd);
        srand(seed);
    }
    return ret ? CORE_STATUS_OK : CORE_STATUS_INTERNAL_ERROR;
}

static core_status_t SERVER_APP_CALL
server_app_init_random(void)
{
    if (server_app_random_init_kern("/dev/urandom") == CORE_STATUS_OK)
        return CORE_STATUS_OK;
    if (server_app_random_init_kern("/dev/random") == CORE_STATUS_OK)
        return CORE_STATUS_OK;
    if (server_app_random_init_kern("/dev/arandom") == CORE_STATUS_OK)
        return CORE_STATUS_OK;
    srand(rand() ^ (unsigned)getpid() ^ (unsigned)time(NULL));
    return CORE_STATUS_OK;
}

static core_status_t SERVER_APP_CALL
server_app_init_preloop(int syslog_mode)
{
    if (!s_config_api || !s_config_api->init
        || s_config_api->init() != CORE_STATUS_OK)
        return CORE_STATUS_INTERNAL_ERROR;
    if (!s_logging_api || !s_logging_api->init || !s_logging_api->reinit)
        return CORE_STATUS_INTERNAL_ERROR;
    if (s_logging_api->init(syslog_mode) != CORE_STATUS_OK)
        return CORE_STATUS_INTERNAL_ERROR;
    s_logging_api->reinit();
    return CORE_STATUS_OK;
}

static core_status_t SERVER_APP_CALL
server_app_init_runtime_loop(void)
{
    if (!s_transport_api || !s_transport_api->init
        || !s_transport_api->init_connections)
        return CORE_STATUS_INTERNAL_ERROR;
    if (s_transport_api->init(CONNECTION_POOL) != CORE_STATUS_OK)
        return CORE_STATUS_INTERNAL_ERROR;
    if (!Signals_Init())
        return CORE_STATUS_INTERNAL_ERROR;
    Channel_Init();
    Class_Init();
    Client_Init();
    if (s_transport_api->init_connections() != CORE_STATUS_OK)
        return CORE_STATUS_INTERNAL_ERROR;
    return CORE_STATUS_OK;
}

static core_status_t SERVER_APP_CALL
server_app_build_proto_id(char *buffer, size_t buffer_size)
{
    if (buffer == NULL || buffer_size == 0)
        return CORE_STATUS_INVALID_ARGUMENT;

    config_bootstrap_t bootstrap;
    if (server_app_get_bootstrap(&bootstrap) != CORE_STATUS_OK)
        return CORE_STATUS_INTERNAL_ERROR;

#ifdef IRCPLUS
    snprintf(buffer, buffer_size, "%s%s %s|%s:%s",
             PROTOVER, PROTOIRCPLUS, PACKAGE_NAME, PACKAGE_VERSION,
             IRCPLUSFLAGS);
#ifdef ZLIB
    strlcat(buffer, "Z", buffer_size);
#endif
    if (bootstrap.oper_can_mode)
        strlcat(buffer, "o", buffer_size);
#else
    snprintf(buffer, buffer_size, "%s%s %s|%s",
             PROTOVER, PROTOIRC, PACKAGE_NAME, PACKAGE_VERSION);
#endif
    strlcat(buffer, " P", buffer_size);
#ifdef ZLIB
    strlcat(buffer, "Z", buffer_size);
#endif
    return CORE_STATUS_OK;
}

static core_status_t SERVER_APP_CALL
server_app_init_listeners(void)
{
    if (!s_transport_api || !s_transport_api->init_listeners)
        return CORE_STATUS_INTERNAL_ERROR;
    return s_transport_api->init_listeners();
}

static core_status_t SERVER_APP_CALL
server_app_init_predefined_channels(void)
{
    Channel_InitPredefined();
    return CORE_STATUS_OK;
}

static core_status_t SERVER_APP_CALL
server_app_create_pidfile(void)
{
    config_bootstrap_t bootstrap;
    int pidfd;
    char pidbuf[64];
    int len;
    pid_t pid = getpid();
    const char *pidfile;

    if (server_app_get_bootstrap(&bootstrap) != CORE_STATUS_OK)
        return CORE_STATUS_INTERNAL_ERROR;

    pidfile = bootstrap.pid_file;
    if (!pidfile || !pidfile[0])
        return CORE_STATUS_OK;

    if (s_logging_api && s_logging_api->debug)
        s_logging_api->debug("Creating PID file (%s) ...", pidfile);

    pidfd = open(pidfile, O_RDWR|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (pidfd < 0) {
        if (s_logging_api && s_logging_api->log)
            s_logging_api->log(LOG_ERR, "Error writing PID file (%s): %s", pidfile, strerror(errno));
        return CORE_STATUS_INTERNAL_ERROR;
    }

    len = snprintf(pidbuf, sizeof pidbuf, "%ld\n", (long)pid);
    if (len < 0 || len >= (int)sizeof pidbuf) {
        if (s_logging_api && s_logging_api->log)
            s_logging_api->log(LOG_ERR, "Error converting process ID!");
        close(pidfd);
        return CORE_STATUS_INTERNAL_ERROR;
    }

    if (write(pidfd, pidbuf, (size_t)len) != (ssize_t)len) {
        if (s_logging_api && s_logging_api->log)
            s_logging_api->log(LOG_ERR, "Can't write PID file (%s): %s!", pidfile, strerror(errno));
    }
    if (close(pidfd) != 0) {
        if (s_logging_api && s_logging_api->log)
            s_logging_api->log(LOG_ERR, "Error closing PID file (%s): %s!", pidfile, strerror(errno));
    }
    return CORE_STATUS_OK;
}

static core_status_t SERVER_APP_CALL
server_app_run_loop(void)
{
    if (!s_transport_api || !s_transport_api->run)
        return CORE_STATUS_INTERNAL_ERROR;
    return s_transport_api->run();
}

static core_status_t SERVER_APP_CALL
server_app_delete_pidfile(void)
{
    config_bootstrap_t bootstrap;
    const char *pidfile;

    if (server_app_get_bootstrap(&bootstrap) != CORE_STATUS_OK)
        return CORE_STATUS_INTERNAL_ERROR;

    pidfile = bootstrap.pid_file;
    if (!pidfile || !pidfile[0])
        return CORE_STATUS_OK;

    if (s_logging_api && s_logging_api->debug)
        s_logging_api->debug("Removing PID file (%s) ...", pidfile);
    if (unlink(pidfile)) {
        if (s_logging_api && s_logging_api->log)
            s_logging_api->log(LOG_ERR, "Error unlinking PID file (%s): %s", pidfile, strerror(errno));
    }
    return CORE_STATUS_OK;
}

static core_status_t SERVER_APP_CALL
server_app_shutdown_runtime(void)
{
    if (s_transport_api && s_transport_api->close_all_sockets)
        s_transport_api->close_all_sockets(-1);
    if (s_transport_api && s_transport_api->shutdown_listeners)
        s_transport_api->shutdown_listeners();
    if (s_transport_api && s_transport_api->shutdown_connections)
        s_transport_api->shutdown_connections();
    Client_Exit();
    Channel_Exit();
    Class_Exit();
    if (s_logging_api && s_logging_api->shutdown)
        s_logging_api->shutdown();
    Signals_Exit();
    return CORE_STATUS_OK;
}

static core_status_t SERVER_APP_CALL
server_app_shutdown_runtime_loop(void)
{
    if (s_transport_api && s_transport_api->shutdown)
        s_transport_api->shutdown();
    return CORE_STATUS_OK;
}

static core_status_t SERVER_APP_CALL
server_app_init_daemon(int nodaemon)
{
    if (!NGIRCd_Init(nodaemon != 0))
        return CORE_STATUS_INTERNAL_ERROR;
    return CORE_STATUS_OK;
}

static const server_app_api_t ServerAppApi = {
    1u,
    0u,
    server_app_create,
    server_app_start,
    server_app_stop,
    server_app_init_runtime,
    server_app_init_random,
    server_app_init_preloop,
    server_app_init_runtime_loop,
    server_app_build_proto_id,
    server_app_init_predefined_channels,
    server_app_init_listeners,
    server_app_create_pidfile,
    server_app_run_loop,
    server_app_shutdown_runtime,
    server_app_shutdown_runtime_loop,
    server_app_delete_pidfile,
    server_app_init_daemon
};

SERVER_APP_API const server_app_api_t * SERVER_APP_CALL
server_app_get_api_v1(void)
{
    return &ServerAppApi;
}

static const core_module_metadata_t ServerAppMetadata = {
    1u, 0u, CORE_MODULE_KIND_SERVER_APP, "server_app", "1.0"
};

static const core_module_api_t ServerAppModuleApi = {
    1u, 0u, &ServerAppMetadata, &ServerAppApi
};

SERVER_APP_API const core_module_api_t * SERVER_APP_CALL
module_get_api_v1(void)
{
    return &ServerAppModuleApi;
}

