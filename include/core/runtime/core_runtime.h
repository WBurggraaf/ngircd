#ifndef CORE_RUNTIME_CORE_RUNTIME_H
#define CORE_RUNTIME_CORE_RUNTIME_H

#include <stddef.h>
#include <stdint.h>

#ifdef _WIN32
#  ifdef CORE_RUNTIME_BUILD
#    define CORE_RUNTIME_API __declspec(dllexport)
#  else
#    define CORE_RUNTIME_API __declspec(dllimport)
#  endif
#  define CORE_RUNTIME_CALL __cdecl
#else
#  define CORE_RUNTIME_API
#  define CORE_RUNTIME_CALL
#endif

#define CORE_RUNTIME_API_VERSION_MAJOR 1u
#define CORE_RUNTIME_API_VERSION_MINOR 0u

typedef enum core_status {
    CORE_STATUS_OK = 0,
    CORE_STATUS_INVALID_ARGUMENT = 1,
    CORE_STATUS_OUT_OF_MEMORY = 2,
    CORE_STATUS_NOT_FOUND = 3,
    CORE_STATUS_UNSUPPORTED = 4,
    CORE_STATUS_IO_ERROR = 5,
    CORE_STATUS_INTERNAL_ERROR = 6
} core_status_t;

typedef struct core_version {
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
} core_version_t;

typedef struct core_buffer {
    void *data;
    size_t size;
} core_buffer_t;

typedef struct core_string_view {
    const char *data;
    size_t size;
} core_string_view_t;

typedef void *(*core_alloc_fn_t)(void *user_data, size_t size, size_t alignment);
typedef void (*core_free_fn_t)(void *user_data, void *ptr);

typedef struct core_allocator {
    void *user_data;
    core_alloc_fn_t allocate;
    core_free_fn_t release;
} core_allocator_t;

typedef struct core_module_metadata {
    uint32_t api_major;
    uint32_t api_minor;
    uint32_t module_kind;
    const char *module_name;
    const char *module_version;
} core_module_metadata_t;

typedef struct core_host_services {
    uint32_t api_major;
    uint32_t api_minor;
    const core_allocator_t *allocator;
} core_host_services_t;

static inline int core_status_succeeded(core_status_t status) {
    return status == CORE_STATUS_OK;
}

static inline int core_status_failed(core_status_t status) {
    return status != CORE_STATUS_OK;
}

static inline core_version_t core_version_make(uint32_t major, uint32_t minor, uint32_t patch) {
    core_version_t version;
    version.major = major;
    version.minor = minor;
    version.patch = patch;
    return version;
}

#ifdef __cplusplus
extern "C" {
#endif

CORE_RUNTIME_API core_version_t CORE_RUNTIME_CALL core_runtime_api_version(void);

#ifdef __cplusplus
}
#endif

#endif
