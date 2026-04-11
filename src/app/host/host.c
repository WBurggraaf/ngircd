#include "app/host/host.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "protocol/irc_protocol/irc_protocol.h"
#include "protocol/command_handlers/command_handlers.h"
#include "state/client_state/client_state.h"
#include "state/channel_state/channel_state.h"
#include "app/server_app/server_app.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

/* ---------------------------------------------------------------------------
 * Internal helpers
 * --------------------------------------------------------------------------*/

#ifdef _WIN32

static HMODULE
load_dll(const char *dll_dir, const char *dll_name,
         char *errbuf, size_t errbuf_size)
{
    char path[MAX_PATH];

    if (dll_dir && dll_dir[0]) {
        snprintf(path, sizeof(path), "%s\\%s", dll_dir, dll_name);
    } else {
        strncpy(path, dll_name, sizeof(path) - 1);
        path[sizeof(path) - 1] = '\0';
    }

    HMODULE h = LoadLibraryA(path);
    if (!h && errbuf && errbuf_size > 0) {
        snprintf(errbuf, errbuf_size,
                 "host_load_modules: LoadLibraryA(\"%s\") failed (error %lu)",
                 path, (unsigned long)GetLastError());
    }
    return h;
}

static core_module_get_api_v1_fn
resolve_module_fn(HMODULE h, const char *dll_name,
                  char *errbuf, size_t errbuf_size)
{
    FARPROC p = GetProcAddress(h, CORE_RUNTIME_MODULE_GET_API_V1);
    if (!p && errbuf && errbuf_size > 0) {
        snprintf(errbuf, errbuf_size,
                 "host_load_modules: GetProcAddress(\"%s\", \"%s\") failed (error %lu)",
                 dll_name, CORE_RUNTIME_MODULE_GET_API_V1,
                 (unsigned long)GetLastError());
    }
    return (core_module_get_api_v1_fn)(void *)p;
}

#endif /* _WIN32 */

static core_status_t
validate_and_extract(const core_module_api_t *mod_api,
                     uint32_t expected_kind, const char *dll_name,
                     const void **api_out,
                     char *errbuf, size_t errbuf_size)
{
    if (!mod_api) {
        if (errbuf && errbuf_size > 0)
            snprintf(errbuf, errbuf_size,
                     "host_load_modules: %s module_get_api_v1 returned NULL",
                     dll_name);
        return CORE_STATUS_INTERNAL_ERROR;
    }
    if (mod_api->api_major != 1u) {
        if (errbuf && errbuf_size > 0)
            snprintf(errbuf, errbuf_size,
                     "host_load_modules: %s api_major=%u (expected 1)",
                     dll_name, mod_api->api_major);
        return CORE_STATUS_UNSUPPORTED;
    }
    if (mod_api->metadata && mod_api->metadata->module_kind != expected_kind) {
        if (errbuf && errbuf_size > 0)
            snprintf(errbuf, errbuf_size,
                     "host_load_modules: %s module_kind=%u (expected %u)",
                     dll_name, mod_api->metadata->module_kind, expected_kind);
        return CORE_STATUS_UNSUPPORTED;
    }
    if (!mod_api->api) {
        if (errbuf && errbuf_size > 0)
            snprintf(errbuf, errbuf_size,
                     "host_load_modules: %s module_get_api_v1()->api is NULL",
                     dll_name);
        return CORE_STATUS_INTERNAL_ERROR;
    }
    *api_out = mod_api->api;
    return CORE_STATUS_OK;
}

/* ---------------------------------------------------------------------------
 * Public: host_load_modules
 * --------------------------------------------------------------------------*/

HOST_API core_status_t HOST_CALL
host_load_modules(const char *dll_dir, host_module_set_t *out,
                  char *errbuf, size_t errbuf_size)
{
    if (!out)
        return CORE_STATUS_INVALID_ARGUMENT;

    memset(out, 0, sizeof(*out));

#ifdef _WIN32
    static const struct {
        const char *name;
        uint32_t    kind;
        size_t      offset; /* byte offset of the pointer in host_module_set_t */
    } modules[] = {
        { "logging.dll",       CORE_MODULE_KIND_LOGGING,       offsetof(host_module_set_t, logging)       },
        { "config.dll",        CORE_MODULE_KIND_CONFIG,        offsetof(host_module_set_t, config)        },
        { "platform.dll",      CORE_MODULE_KIND_PLATFORM,      offsetof(host_module_set_t, platform)      },
        { "net_transport.dll", CORE_MODULE_KIND_NET_TRANSPORT, offsetof(host_module_set_t, net_transport) },
        { "resolver.dll",      CORE_MODULE_KIND_RESOLVER,      offsetof(host_module_set_t, resolver)      },
        { "irc_protocol.dll",  CORE_MODULE_KIND_IRC_PROTOCOL,  offsetof(host_module_set_t, irc_protocol)  },
        { "client_state.dll",  CORE_MODULE_KIND_CLIENT_STATE,  offsetof(host_module_set_t, client_state)  },
        { "channel_state.dll", CORE_MODULE_KIND_CHANNEL_STATE, offsetof(host_module_set_t, channel_state) },
        { "command_handlers.dll", CORE_MODULE_KIND_COMMAND_HANDLERS, offsetof(host_module_set_t, command_handlers) },
        { "server_app.dll",    CORE_MODULE_KIND_SERVER_APP,    offsetof(host_module_set_t, server_app)    },
    };

    for (size_t i = 0; i < sizeof(modules) / sizeof(modules[0]); i++) {
        HMODULE h = load_dll(dll_dir, modules[i].name, errbuf, errbuf_size);
        if (!h)
            return CORE_STATUS_NOT_FOUND;

        core_module_get_api_v1_fn get_api =
            resolve_module_fn(h, modules[i].name, errbuf, errbuf_size);
        if (!get_api)
            return CORE_STATUS_NOT_FOUND;

        const core_module_api_t *mod_api = get_api();
        const void *typed_api = NULL;
        core_status_t st = validate_and_extract(
            mod_api, modules[i].kind, modules[i].name,
            &typed_api, errbuf, errbuf_size);
        if (st != CORE_STATUS_OK)
            return st;

        /* Write the typed API pointer into the correct field of *out */
        memcpy((char *)out + modules[i].offset, &typed_api, sizeof(void *));
    }

    return CORE_STATUS_OK;

#else
    /* Non-Windows: fall back to static link-time resolution.
     * On POSIX the wrapper DLLs (.so) are loaded by the OS loader at startup,
     * so calling the module-specific getters is equivalent. */
    if (errbuf && errbuf_size > 0)
        snprintf(errbuf, errbuf_size,
                 "host_load_modules: dynamic loading not implemented on this platform");
    return CORE_STATUS_UNSUPPORTED;
#endif
}

/* ---------------------------------------------------------------------------
 * Public: host_get_wiring_v1 (validation helper, kept for compatibility)
 * --------------------------------------------------------------------------*/

static core_status_t host_wire_modules(const host_module_set_t *modules)
{
    if (modules == NULL)
        return CORE_STATUS_INVALID_ARGUMENT;
    if (modules->logging == NULL || modules->config == NULL
        || modules->platform == NULL || modules->net_transport == NULL
        || modules->resolver == NULL || modules->irc_protocol == NULL
        || modules->client_state == NULL || modules->channel_state == NULL
        || modules->command_handlers == NULL || modules->server_app == NULL)
        return CORE_STATUS_INVALID_ARGUMENT;

    if (modules->logging->api_major != 1u
        || modules->config->api_major != 1u
        || modules->platform->api_major != 1u
        || modules->net_transport->api_major != 1u
        || modules->resolver->api_major != 1u
        || modules->irc_protocol->api_major != 1u
        || modules->client_state->api_major != 1u
        || modules->channel_state->api_major != 1u
        || modules->command_handlers->api_major != 1u
        || modules->server_app->api_major != 1u)
        return CORE_STATUS_UNSUPPORTED;

    return CORE_STATUS_OK;
}

static const host_wiring_t HostWiring = {
    1u,
    0u,
    host_wire_modules
};

const host_wiring_t *
host_get_wiring_v1(void)
{
    return &HostWiring;
}

