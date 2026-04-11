/*
 * config_module_test.c — unit tests for the config wrapper DLL API
 *
 * Compiles src/config/config.c directly with CONFIG_BUILD.
 * Uses test_support.c for Conf_* globals and conf_func_stubs.c for
 * Conf_Init / Conf_Rehash / Conf_Test stubs.
 *
 * Tests:
 *   - config_get_api_v1()  struct integrity
 *   - config_get_bootstrap() happy path and NULL guard
 *   - module_get_api_v1()  metadata integrity
 */

#include "portab/portab.h"
#include "config/config.h"
#include "core_runtime/module_api.h"

#include <stdio.h>
#include <string.h>

/* Forward-declare module_get_api_v1 compiled from config.c */
extern const core_module_api_t * module_get_api_v1(void);

/* ---- CHECK macro ---- */
#define CHECK(expr, name) \
    do { \
        if (!(expr)) { \
            fprintf(stderr, "FAIL: %s\n", (name)); \
            return 1; \
        } \
    } while (0)

int main(void)
{
    /* ---- config_get_api_v1 struct ---- */
    const config_api_t *api = config_get_api_v1();
    CHECK(api != NULL,                 "config_api_not_null");
    CHECK(api->api_major == 1u,        "config_api_major_1");
    CHECK(api->api_minor == 0u,        "config_api_minor_0");
    CHECK(api->init         != NULL,   "config_init_ptr_not_null");
    CHECK(api->rehash       != NULL,   "config_rehash_ptr_not_null");
    CHECK(api->test         != NULL,   "config_test_ptr_not_null");
    CHECK(api->get_bootstrap != NULL,  "config_get_bootstrap_ptr_not_null");

    /* ---- config_get_api_v1 is idempotent (same pointer) ---- */
    CHECK(config_get_api_v1() == api,  "config_api_idempotent");

    /* ---- get_bootstrap NULL guard ---- */
    CHECK(api->get_bootstrap(NULL) == CORE_STATUS_INVALID_ARGUMENT,
          "config_bootstrap_null_arg");

    /* ---- get_bootstrap happy path ---- */
    config_bootstrap_t bs;
    memset(&bs, 0xFF, sizeof(bs));   /* fill with non-zero to catch no-fill bugs */
    CHECK(api->get_bootstrap(&bs) == CORE_STATUS_OK, "config_bootstrap_ok");
    CHECK(bs.api_version == 1u,        "config_bootstrap_api_version_1");

    /* ---- init and test return OK/valid codes ---- */
    CHECK(api->init()   == CORE_STATUS_OK, "config_init_returns_ok");
    CHECK(api->rehash() == CORE_STATUS_OK, "config_rehash_returns_ok");
    /* Conf_Test returns 0 from stub → config_test wraps as pass-through int */
    CHECK(api->test()   == 0,              "config_test_returns_zero");

    /* ---- module_get_api_v1 metadata ---- */
    const core_module_api_t *m = module_get_api_v1();
    CHECK(m != NULL,                                       "config_mod_api_not_null");
    CHECK(m->api_major == 1u,                              "config_mod_api_major_1");
    CHECK(m->metadata != NULL,                             "config_mod_metadata_not_null");
    CHECK(m->metadata->module_kind == CORE_MODULE_KIND_CONFIG, "config_mod_kind_correct");
    CHECK(m->metadata->module_name != NULL,                "config_mod_name_not_null");
    CHECK(m->metadata->module_name[0] != '\0',             "config_mod_name_nonempty");
    CHECK(m->api != NULL,                                  "config_mod_api_ptr_not_null");
    CHECK(m->api == (const void *)config_get_api_v1(),     "config_mod_api_matches_typed");

    printf("config_module_test: all 21 checks passed\n");
    return 0;
}
