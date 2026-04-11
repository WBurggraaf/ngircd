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
logging_log_message(int level, const char *message)
{
    Log(level, "%s", message ? message : "");
}

static void LOGGING_CALL
logging_debug_message(const char *message)
{
    LogDebug("%s", message ? message : "");
}

static const logging_api_t LoggingApi = {
    1u,
    0u,
    logging_init,
    logging_shutdown,
    logging_reinit,
    logging_log_message,
    logging_debug_message
};

LOGGING_API const logging_api_t * LOGGING_CALL
logging_get_api_v1(void)
{
    return &LoggingApi;
}

CORE_RUNTIME_DECLARE_MODULE_API(CORE_MODULE_KIND_LOGGING, "logging", &LoggingApi);

LOGGING_API const core_module_api_t * LOGGING_CALL
module_get_api_v1(void)
{
    return &module_module_api;
}

