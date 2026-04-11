#include "app/logging/logging.h"

#include <stdio.h>

#include "core/runtime/module_api.h"
#include "log.h"

static core_status_t LOGGING_CALL
logging_init(int syslog_mode)
{
    Log_Init(syslog_mode != 0);
    return CORE_STATUS_OK;
}

static void LOGGING_CALL
logging_shutdown(void)
{
    Log_Exit();
}

static void LOGGING_CALL
logging_reinit(void)
{
    Log_ReInit();
}

static void LOGGING_CALL
logging_log(int level, const char *format, ...)
{
    va_list ap;
    char buffer[1024];

    va_start(ap, format);
    vsnprintf(buffer, sizeof(buffer), format, ap);
    va_end(ap);
    Log(level, "%s", buffer);
}

static void LOGGING_CALL
logging_debug(const char *format, ...)
{
    va_list ap;
    char buffer[1024];

    va_start(ap, format);
    vsnprintf(buffer, sizeof(buffer), format, ap);
    va_end(ap);
    LogDebug("%s", buffer);
}

static const logging_api_t LoggingApi = {
    1u,
    0u,
    logging_init,
    logging_shutdown,
    logging_reinit,
    logging_log,
    logging_debug
};

LOGGING_API const logging_api_t * LOGGING_CALL
logging_get_api_v1(void)
{
    return &LoggingApi;
}

static const core_module_metadata_t LoggingMetadata = {
    1u, 0u, CORE_MODULE_KIND_LOGGING, "logging", "1.0"
};

static const core_module_api_t LoggingModuleApi = {
    1u, 0u, &LoggingMetadata, &LoggingApi
};

LOGGING_API const core_module_api_t * LOGGING_CALL
module_get_api_v1(void)
{
    return &LoggingModuleApi;
}

