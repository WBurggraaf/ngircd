#include "core_runtime/core_runtime.h"

CORE_RUNTIME_API core_version_t CORE_RUNTIME_CALL
core_runtime_api_version(void)
{
    return core_version_make(
        CORE_RUNTIME_API_VERSION_MAJOR,
        CORE_RUNTIME_API_VERSION_MINOR,
        0u);
}
