#include "platform/platform.h"

#include <fcntl.h>

#include "core_runtime/module_api.h"

#ifdef _WIN32
#include "config.h"
#include "win32compat.h"
#else
#include <unistd.h>
#endif

static core_status_t PLATFORM_CALL
platform_open_file(const char *path, int flags)
{
    return open(path, flags) >= 0 ? CORE_STATUS_OK : CORE_STATUS_IO_ERROR;
}

static core_status_t PLATFORM_CALL
platform_close_file(int fd)
{
    return close(fd) == 0 ? CORE_STATUS_OK : CORE_STATUS_IO_ERROR;
}

static core_status_t PLATFORM_CALL
platform_set_nonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL);
    if (flags < 0)
        return CORE_STATUS_IO_ERROR;
#ifndef O_NONBLOCK
#define O_NONBLOCK O_NDELAY
#endif
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0 ? CORE_STATUS_OK : CORE_STATUS_IO_ERROR;
}

static core_status_t PLATFORM_CALL
platform_set_cloexec(int fd)
{
    int flags = fcntl(fd, F_GETFD);
    if (flags < 0)
        return CORE_STATUS_IO_ERROR;
#ifdef FD_CLOEXEC
    flags |= FD_CLOEXEC;
#endif
    return fcntl(fd, F_SETFD, flags) == 0 ? CORE_STATUS_OK : CORE_STATUS_IO_ERROR;
}

static const platform_api_t PlatformApi = {
    1u,
    0u,
    platform_open_file,
    platform_close_file,
    platform_set_nonblock,
    platform_set_cloexec
};

PLATFORM_API const platform_api_t * PLATFORM_CALL
platform_get_api_v1(void)
{
    return &PlatformApi;
}

static const core_module_metadata_t PlatformMetadata = {
    1u, 0u, CORE_MODULE_KIND_PLATFORM, "platform", "1.0"
};

static const core_module_api_t PlatformModuleApi = {
    1u, 0u, &PlatformMetadata, &PlatformApi
};

PLATFORM_API const core_module_api_t * PLATFORM_CALL
module_get_api_v1(void)
{
    return &PlatformModuleApi;
}
