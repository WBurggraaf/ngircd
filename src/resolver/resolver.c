#include "resolver/resolver.h"

#include "portab/portab.h"
#include "ngircd/proc.h"
#include "ipaddr/ng_ipaddr.h"
#include "ngircd/resolve.h"

static core_status_t RESOLVER_CALL
resolver_resolve_name(const char *host)
{
    PROC_STAT proc;

    Proc_InitStruct(&proc);
    return Resolve_Name(&proc, host, NULL) ? CORE_STATUS_OK : CORE_STATUS_INTERNAL_ERROR;
}

static core_status_t RESOLVER_CALL
resolver_resolve_addr_ident(const void *addr, uint32_t addr_size)
{
    PROC_STAT proc;

    if (addr == NULL || addr_size < sizeof(ng_ipaddr_t))
        return CORE_STATUS_INVALID_ARGUMENT;

    Proc_InitStruct(&proc);
    return Resolve_Addr_Ident(&proc, (const ng_ipaddr_t *)addr, -1, NULL) ? CORE_STATUS_OK : CORE_STATUS_INTERNAL_ERROR;
}

static const resolver_api_t ResolverApi = {
    1u,
    0u,
    resolver_resolve_name,
    resolver_resolve_addr_ident
};

RESOLVER_API const resolver_api_t * RESOLVER_CALL
resolver_get_api_v1(void)
{
    return &ResolverApi;
}
