#include "server_app/server_app.h"

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

#include "portab/portab.h"
#include "ngircd/defines.h"
#include "ngircd/ngircd.h"
#include "config/config.h"
#include "logging/logging.h"
#include "net_transport/net_transport.h"

static core_status_t SERVER_APP_CALL
server_app_create(void)
{
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
    const config_api_t *config_api = config_get_api_v1();

    if (!config_api || !config_api->get_bootstrap || !bootstrap)
        return CORE_STATUS_INTERNAL_ERROR;
    return config_api->get_bootstrap(bootstrap);
}

static core_status_t SERVER_APP_CALL
server_app_init_runtime(void)
{
    NGIRCd_Start = time(NULL);
    (void)strftime(NGIRCd_StartStr, sizeof(NGIRCd_StartStr),
                   "%a %b %d %Y at %H:%M:%S (%Z)",
                   localtime(&NGIRCd_Start));
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
    const config_api_t *config_api = config_get_api_v1();
    const logging_api_t *logging_api = logging_get_api_v1();

    if (!config_api || !config_api->init
        || config_api->init() != CORE_STATUS_OK)
        return CORE_STATUS_INTERNAL_ERROR;
    if (!logging_api || !logging_api->init || !logging_api->reinit)
        return CORE_STATUS_INTERNAL_ERROR;
    if (logging_api->init(syslog_mode) != CORE_STATUS_OK)
        return CORE_STATUS_INTERNAL_ERROR;
    logging_api->reinit();
    return CORE_STATUS_OK;
}

static core_status_t SERVER_APP_CALL
server_app_init_runtime_loop(void)
{
    const net_transport_api_t *transport_api = net_transport_get_api_v1();

    if (!transport_api || !transport_api->init
        || !transport_api->init_connections)
        return CORE_STATUS_INTERNAL_ERROR;
    if (transport_api->init(CONNECTION_POOL) != CORE_STATUS_OK)
        return CORE_STATUS_INTERNAL_ERROR;
    if (!Signals_Init())
        return CORE_STATUS_INTERNAL_ERROR;
    Channel_Init();
    Class_Init();
    Client_Init();
    if (transport_api->init_connections() != CORE_STATUS_OK)
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
    const net_transport_api_t *transport_api = net_transport_get_api_v1();

    if (!transport_api || !transport_api->init_listeners)
        return CORE_STATUS_INTERNAL_ERROR;
    return transport_api->init_listeners();
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
    const logging_api_t *logging_api = logging_get_api_v1();
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

    if (logging_api && logging_api->debug)
        logging_api->debug("Creating PID file (%s) ...", pidfile);

    pidfd = open(pidfile, O_RDWR|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (pidfd < 0) {
        if (logging_api && logging_api->log)
            logging_api->log(LOG_ERR, "Error writing PID file (%s): %s", pidfile, strerror(errno));
        return CORE_STATUS_INTERNAL_ERROR;
    }

    len = snprintf(pidbuf, sizeof pidbuf, "%ld\n", (long)pid);
    if (len < 0 || len >= (int)sizeof pidbuf) {
        if (logging_api && logging_api->log)
            logging_api->log(LOG_ERR, "Error converting process ID!");
        close(pidfd);
        return CORE_STATUS_INTERNAL_ERROR;
    }

    if (write(pidfd, pidbuf, (size_t)len) != (ssize_t)len) {
        if (logging_api && logging_api->log)
            logging_api->log(LOG_ERR, "Can't write PID file (%s): %s!", pidfile, strerror(errno));
    }
    if (close(pidfd) != 0) {
        if (logging_api && logging_api->log)
            logging_api->log(LOG_ERR, "Error closing PID file (%s): %s!", pidfile, strerror(errno));
    }
    return CORE_STATUS_OK;
}

static core_status_t SERVER_APP_CALL
server_app_run_loop(void)
{
    const net_transport_api_t *transport_api = net_transport_get_api_v1();

    if (!transport_api || !transport_api->run)
        return CORE_STATUS_INTERNAL_ERROR;
    return transport_api->run();
}

static core_status_t SERVER_APP_CALL
server_app_delete_pidfile(void)
{
    config_bootstrap_t bootstrap;
    const logging_api_t *logging_api = logging_get_api_v1();
    const char *pidfile;

    if (server_app_get_bootstrap(&bootstrap) != CORE_STATUS_OK)
        return CORE_STATUS_INTERNAL_ERROR;

    pidfile = bootstrap.pid_file;
    if (!pidfile || !pidfile[0])
        return CORE_STATUS_OK;

    if (logging_api && logging_api->debug)
        logging_api->debug("Removing PID file (%s) ...", pidfile);
    if (unlink(pidfile)) {
        if (logging_api && logging_api->log)
            logging_api->log(LOG_ERR, "Error unlinking PID file (%s): %s", pidfile, strerror(errno));
    }
    return CORE_STATUS_OK;
}

static core_status_t SERVER_APP_CALL
server_app_shutdown_runtime(void)
{
    const logging_api_t *logging_api = logging_get_api_v1();
    const net_transport_api_t *transport_api = net_transport_get_api_v1();

    if (transport_api && transport_api->close_all_sockets)
        transport_api->close_all_sockets(-1);
    if (transport_api && transport_api->shutdown_listeners)
        transport_api->shutdown_listeners();
    if (transport_api && transport_api->shutdown_connections)
        transport_api->shutdown_connections();
    Client_Exit();
    Channel_Exit();
    Class_Exit();
    if (logging_api && logging_api->shutdown)
        logging_api->shutdown();
    Signals_Exit();
    return CORE_STATUS_OK;
}

static core_status_t SERVER_APP_CALL
server_app_shutdown_runtime_loop(void)
{
    const net_transport_api_t *transport_api = net_transport_get_api_v1();

    if (transport_api && transport_api->shutdown)
        transport_api->shutdown();
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
