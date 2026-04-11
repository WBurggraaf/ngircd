cmake_minimum_required(VERSION 3.15)
project(ngircd C)

set(CMAKE_C_STANDARD 99)
include(CTest)

# Source directories
set(ORIG_SRC "${CMAKE_SOURCE_DIR}/original/src")
set(WIN32_SRC "${CMAKE_SOURCE_DIR}/win32/src")
set(WIN32_INC "${CMAKE_SOURCE_DIR}/win32/include")
set(SHARED_INC "${CMAKE_SOURCE_DIR}/include")
set(SHARED_SRC "${CMAKE_SOURCE_DIR}/src")

# ---------------------------------------------------------------------------
# Support static libraries (no export concerns)
# ---------------------------------------------------------------------------

add_library(ngportab STATIC
    ${ORIG_SRC}/portab/strdup.c
    ${ORIG_SRC}/portab/strlcpy.c
    ${ORIG_SRC}/portab/strndup.c
    ${ORIG_SRC}/portab/strtok_r.c
    ${ORIG_SRC}/portab/vsnprintf.c
)
target_include_directories(ngportab PUBLIC
    ${WIN32_INC} ${SHARED_INC} ${ORIG_SRC}/portab)
target_compile_definitions(ngportab PRIVATE
    _CRT_SECURE_NO_WARNINGS _WINSOCK_DEPRECATED_NO_WARNINGS)

add_library(ngtool STATIC ${ORIG_SRC}/tool/tool.c)
target_include_directories(ngtool PUBLIC
    ${WIN32_INC} ${SHARED_INC} ${ORIG_SRC}/portab ${ORIG_SRC}/tool)
target_compile_definitions(ngtool PRIVATE
    _CRT_SECURE_NO_WARNINGS _WINSOCK_DEPRECATED_NO_WARNINGS)

add_library(ngipaddr STATIC ${ORIG_SRC}/ipaddr/ng_ipaddr.c)
target_include_directories(ngipaddr PUBLIC
    ${WIN32_INC} ${SHARED_INC} ${ORIG_SRC}/portab ${ORIG_SRC}/ipaddr)
target_compile_definitions(ngipaddr PRIVATE
    _CRT_SECURE_NO_WARNINGS _WINSOCK_DEPRECATED_NO_WARNINGS)
target_link_libraries(ngipaddr PUBLIC ws2_32)

# ---------------------------------------------------------------------------
# core_runtime.dll  — pure ABI contracts, no legacy deps, C# P/Invoke ready
# ---------------------------------------------------------------------------

add_library(core_runtime SHARED
    ${SHARED_SRC}/core_runtime/core_runtime.c)
target_include_directories(core_runtime PUBLIC ${SHARED_INC})
target_compile_definitions(core_runtime PRIVATE CORE_RUNTIME_BUILD)
set_target_properties(core_runtime PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

# ---------------------------------------------------------------------------
# platform.dll  — OS primitives, no legacy deps, C# P/Invoke ready
# ---------------------------------------------------------------------------

add_library(platform SHARED ${SHARED_SRC}/platform/platform.c)
target_include_directories(platform PUBLIC ${SHARED_INC} PRIVATE ${WIN32_INC})
target_compile_definitions(platform PRIVATE PLATFORM_BUILD)
target_link_libraries(platform PUBLIC core_runtime)
set_target_properties(platform PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

# ---------------------------------------------------------------------------
# Stub DLLs — no legacy deps, C# P/Invoke ready today
# ---------------------------------------------------------------------------

add_library(irc_protocol SHARED ${SHARED_SRC}/irc_protocol/irc_protocol.c)
target_include_directories(irc_protocol PUBLIC ${SHARED_INC})
target_compile_definitions(irc_protocol PRIVATE IRC_PROTOCOL_BUILD)
target_link_libraries(irc_protocol PUBLIC core_runtime)
set_target_properties(irc_protocol PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(client_state SHARED ${SHARED_SRC}/client_state/client_state.c)
target_include_directories(client_state PUBLIC ${SHARED_INC})
target_compile_definitions(client_state PRIVATE CLIENT_STATE_BUILD)
target_link_libraries(client_state PUBLIC core_runtime)
set_target_properties(client_state PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(channel_state SHARED ${SHARED_SRC}/channel_state/channel_state.c)
target_include_directories(channel_state PUBLIC ${SHARED_INC})
target_compile_definitions(channel_state PRIVATE CHANNEL_STATE_BUILD)
target_link_libraries(channel_state PUBLIC core_runtime)
set_target_properties(channel_state PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(command_handlers SHARED
    ${SHARED_SRC}/command_handlers/command_handlers.c)
target_include_directories(command_handlers PUBLIC ${SHARED_INC})
target_compile_definitions(command_handlers PRIVATE COMMAND_HANDLERS_BUILD)
target_link_libraries(command_handlers PUBLIC core_runtime)
set_target_properties(command_handlers PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

# nghost target → builds host.dll / host.lib.
# We use the CMake target name "nghost" (not "host") to avoid a collision with
# the NMake reserved pseudo-target "host", which causes U1073 errors when a
# dependent target lists host.lib as a prerequisite.
add_library(nghost SHARED ${SHARED_SRC}/host/host.c)
set_target_properties(nghost PROPERTIES OUTPUT_NAME "host")
target_include_directories(nghost PUBLIC ${SHARED_INC}
    PRIVATE ${WIN32_INC} ${ORIG_SRC} ${ORIG_SRC}/ngircd ${ORIG_SRC}/portab)
target_compile_definitions(nghost PRIVATE HOST_BUILD)
target_link_libraries(nghost PUBLIC core_runtime)
set_target_properties(nghost PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

# ---------------------------------------------------------------------------
# ngircd_impl.dll  — the complete original monolith as a DLL.
#
# Architecture:
#   - Contains all legacy C sources EXCEPT ngircd.c (the entry point).
#   - ngircd_runtime_globals.c  defines NGIRCd_* global variables (dllexport).
#   - ngircd_daemon.c           defines NGIRCd_Init and daemon helpers.
#   - WINDOWS_EXPORT_ALL_SYMBOLS auto-exports all C functions via a .def file.
#   - No dependencies on any wrapper DLL → zero circular dependencies.
#
# Wrapper DLLs (logging, config, …) link this for their implementation.
# The EXE imports NGIRCd_* globals and functions from here via dllimport.
# ---------------------------------------------------------------------------

add_library(ngircd_impl SHARED
    # Globals and daemon bootstrap (must be in this DLL for dllexport)
    ${SHARED_SRC}/ngircd_impl/ngircd_runtime_globals.c
    ${SHARED_SRC}/ngircd_impl/ngircd_daemon.c
    # Phase 2 shim: forwards Conn_*/io_* to net_transport.dll at runtime
    ${SHARED_SRC}/ngircd_impl/conn_shim.c
    # Original monolith (minus ngircd.c which is the EXE entry)
    ${ORIG_SRC}/ngircd/core/array.c
    ${ORIG_SRC}/ngircd/channel.c
    ${ORIG_SRC}/ngircd/core/class.c
    ${ORIG_SRC}/ngircd/core/client.c
    ${ORIG_SRC}/ngircd/core/client-cap.c
    ${ORIG_SRC}/ngircd/core/conf.c
    ${ORIG_SRC}/ngircd/core/hash.c
    ${ORIG_SRC}/ngircd/proto/irc.c
    ${ORIG_SRC}/ngircd/proto/irc-cap.c
    ${ORIG_SRC}/ngircd/proto/irc-channel.c
    ${ORIG_SRC}/ngircd/proto/irc-encoding.c
    ${ORIG_SRC}/ngircd/proto/irc-info.c
    ${ORIG_SRC}/ngircd/proto/irc-login.c
    ${ORIG_SRC}/ngircd/proto/irc-metadata.c
    ${ORIG_SRC}/ngircd/proto/irc-mode.c
    ${ORIG_SRC}/ngircd/proto/irc-op.c
    ${ORIG_SRC}/ngircd/proto/irc-oper.c
    ${ORIG_SRC}/ngircd/proto/irc-server.c
    ${ORIG_SRC}/ngircd/proto/irc-write.c
    ${ORIG_SRC}/ngircd/proto/join_check.c
    ${ORIG_SRC}/ngircd/proto/join_error.c
    ${ORIG_SRC}/ngircd/proto/login_flow.c
    ${ORIG_SRC}/ngircd/proto/login_state.c
    ${ORIG_SRC}/ngircd/proto/login_check.c
    ${ORIG_SRC}/ngircd/proto/nick_check.c
    ${ORIG_SRC}/ngircd/proto/message_check.c
    ${ORIG_SRC}/ngircd/proto/user_check.c
    ${ORIG_SRC}/ngircd/core/lists.c
    ${ORIG_SRC}/ngircd/core/log.c
    ${ORIG_SRC}/ngircd/core/login.c
    ${ORIG_SRC}/ngircd/core/match.c
    ${ORIG_SRC}/ngircd/core/numeric.c
    ${ORIG_SRC}/ngircd/core/op.c
    ${ORIG_SRC}/ngircd/proto/parse.c
    ${ORIG_SRC}/ngircd/core/module_iface.c
    ${WIN32_SRC}/win32_proc.c
    ${WIN32_SRC}/win32_resolve.c
    ${WIN32_SRC}/win32_sighandlers.c
)

set_target_properties(ngircd_impl PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

# All ngircd_impl sources get NGIRCD_IMPL_BUILD so ngircd.h uses dllexport
target_include_directories(ngircd_impl PRIVATE
    ${WIN32_INC}
    ${SHARED_INC}
    ${ORIG_SRC}/portab
    ${ORIG_SRC}/tool
    ${ORIG_SRC}/ipaddr
    ${ORIG_SRC}/ngircd
)
target_compile_definitions(ngircd_impl PRIVATE
    NGIRCD_IMPL_BUILD
    _CRT_SECURE_NO_WARNINGS
    _WINSOCK_DEPRECATED_NO_WARNINGS
    # ngircd_daemon.c uses config_get_api_v1 → needs CONFIG_BUILD absent (dllimport)
    # That is the default when CONFIG_BUILD is not defined.
)
target_link_libraries(ngircd_impl PRIVATE
    ngportab ngtool ngipaddr ws2_32
    core_runtime platform
)

if(MSVC)
    target_compile_options(ngircd_impl PRIVATE
        /W3 /Zi
        /wd4005 /wd4013 /wd4996 /wd4267 /wd4244 /wd4018 /wd4133 /wd4090
    )
    target_link_options(ngircd_impl PRIVATE /DEBUG)
endif()

# ---------------------------------------------------------------------------
# Wrapper DLLs — thin C ABI facades over ngircd_impl; C# P/Invoke ready
#
# Dependency chain (no cycles):
#   core_runtime  ← nothing
#   platform      ← core_runtime
#   ngircd_impl   ← core_runtime, platform, ngportab, ngtool, ngipaddr
#   logging       ← core_runtime, ngircd_impl
#   config        ← core_runtime, ngircd_impl
#   net_transport ← core_runtime, ngircd_impl
#   resolver      ← core_runtime, ngircd_impl
#   server_app    ← core_runtime, ngircd_impl, logging, config, net_transport
#   ngircd.exe    ← ngircd_impl + all wrapper DLLs
# ---------------------------------------------------------------------------

add_library(logging SHARED ${SHARED_SRC}/logging/logging.c)
target_include_directories(logging PUBLIC ${SHARED_INC}
    PRIVATE ${WIN32_INC} ${ORIG_SRC} ${ORIG_SRC}/ngircd ${ORIG_SRC}/portab)
target_compile_definitions(logging PRIVATE LOGGING_BUILD NGIRCD_IMPL_CONSUMER)
target_link_libraries(logging PUBLIC core_runtime PRIVATE ngircd_impl)

add_library(config SHARED ${SHARED_SRC}/config/config.c)
target_include_directories(config PUBLIC ${SHARED_INC}
    PRIVATE ${WIN32_INC} ${ORIG_SRC} ${ORIG_SRC}/ngircd
            ${ORIG_SRC}/ipaddr ${ORIG_SRC}/tool ${ORIG_SRC}/portab)
target_compile_definitions(config PRIVATE CONFIG_BUILD NGIRCD_IMPL_CONSUMER)
target_link_libraries(config PUBLIC core_runtime PRIVATE ngircd_impl)

add_library(net_transport SHARED
    ${SHARED_SRC}/net_transport/net_transport.c
    ${ORIG_SRC}/ngircd/net/conn.c
    ${ORIG_SRC}/ngircd/net/conn-encoding.c
    ${ORIG_SRC}/ngircd/net/conn-func.c
    ${ORIG_SRC}/ngircd/net/io.c
)
target_include_directories(net_transport PUBLIC ${SHARED_INC}
    PRIVATE ${WIN32_INC} ${ORIG_SRC} ${ORIG_SRC}/ngircd
            ${ORIG_SRC}/portab ${ORIG_SRC}/tool ${ORIG_SRC}/ipaddr)
target_compile_definitions(net_transport PRIVATE NET_TRANSPORT_BUILD NGIRCD_IMPL_CONSUMER
    _CRT_SECURE_NO_WARNINGS _WINSOCK_DEPRECATED_NO_WARNINGS)
target_link_libraries(net_transport PUBLIC core_runtime
    PRIVATE ngircd_impl ngportab ngtool ngipaddr ws2_32)
if(MSVC)
    target_compile_options(net_transport PRIVATE
        /W3 /Zi /wd4005 /wd4013 /wd4996 /wd4267 /wd4244 /wd4018 /wd4133 /wd4090)
    target_link_options(net_transport PRIVATE /DEBUG)
endif()

add_library(resolver SHARED ${SHARED_SRC}/resolver/resolver.c)
target_include_directories(resolver PUBLIC ${SHARED_INC}
    PRIVATE ${WIN32_INC} ${ORIG_SRC} ${ORIG_SRC}/ngircd
            ${ORIG_SRC}/ipaddr ${ORIG_SRC}/portab)
target_compile_definitions(resolver PRIVATE RESOLVER_BUILD NGIRCD_IMPL_CONSUMER)
target_link_libraries(resolver PUBLIC core_runtime PRIVATE ngircd_impl)

add_library(server_app SHARED ${SHARED_SRC}/server_app/server_app.c)
target_include_directories(server_app PUBLIC ${SHARED_INC}
    PRIVATE ${WIN32_INC} ${ORIG_SRC} ${ORIG_SRC}/ngircd ${ORIG_SRC}/portab)
target_compile_definitions(server_app PRIVATE SERVER_APP_BUILD NGIRCD_IMPL_CONSUMER)
target_link_libraries(server_app PUBLIC core_runtime
    PRIVATE ngircd_impl logging config net_transport ngportab ngtool)

# ---------------------------------------------------------------------------
# ngircd.exe — thin Windows entry point; all logic in DLLs.
#
# ngircd.c is the only monolith file compiled into the EXE.  It imports
# NGIRCd_* globals and NGIRCd_Init from ngircd_impl.dll via dllimport
# (no NGIRCD_IMPL_BUILD defined = consumer mode).
# ---------------------------------------------------------------------------

add_executable(ngircd
    ${ORIG_SRC}/ngircd/platform/ngircd.c
    ${WIN32_SRC}/win32_main.c
)
# Rename main() in ngircd.c so win32_main.c can provide the real main()
set_source_files_properties(${ORIG_SRC}/ngircd/platform/ngircd.c PROPERTIES
    COMPILE_DEFINITIONS "main=ngircd_main")
target_include_directories(ngircd PRIVATE
    ${WIN32_INC}
    ${SHARED_INC}
    ${ORIG_SRC}/portab
    ${ORIG_SRC}/tool
    ${ORIG_SRC}/ipaddr
    ${ORIG_SRC}/ngircd
)
target_compile_definitions(ngircd PRIVATE
    _CRT_SECURE_NO_WARNINGS
    _WINSOCK_DEPRECATED_NO_WARNINGS
    # NGIRCD_EXE_BUILD: keeps GLOBAL=extern (so function defs in ngircd.c compile)
    # but sets NGIRCD_VAR/NGIRCD_FUNC to dllimport (for NGIRCd_* data vars).
    # We cannot use NGIRCD_IMPL_CONSUMER here because ngircd.c defines GLOBAL
    # functions (main, Fill_Version, etc.) — dllimport on definitions is C2491.
    NGIRCD_EXE_BUILD
)
target_link_libraries(ngircd PRIVATE
    ngircd_impl
    server_app
    core_runtime
    ngportab ngtool ngipaddr
    ws2_32
)
# Link host.lib for host_load_modules and host_get_wiring_v1.
# (logging, config, platform, net_transport, resolver are loaded dynamically)
# Link host.lib via linker flag rather than CMake target to avoid NMake's
# reserved "host" pseudo-target issue (U1073 "don't know how to make 'host.lib'").
# add_dependencies ensures nghost is built before ngircd links.
target_link_options(ngircd PRIVATE "$<TARGET_LINKER_FILE:nghost>")
add_dependencies(ngircd nghost)

if(MSVC)
    target_compile_options(ngportab PRIVATE /W3 /Zi /wd4996 /wd4005)
    target_compile_options(ngtool   PRIVATE /W3 /Zi /wd4996 /wd4005)
    target_compile_options(ngipaddr PRIVATE /W3 /Zi /wd4996 /wd4267 /wd4005)
    target_compile_options(ngircd PRIVATE
        /W3 /Zi
        /wd4005 /wd4013 /wd4996 /wd4267 /wd4244 /wd4018 /wd4133 /wd4090)
    target_link_options(ngircd PRIVATE /DEBUG)
endif()

# ---------------------------------------------------------------------------
# Tests — compile monolith sources directly; independent of ngircd_impl.dll
# ---------------------------------------------------------------------------

if(BUILD_TESTING)
    add_executable(module_iface_test
        tests/module_iface_test.c
        ${ORIG_SRC}/ngircd/core/module_iface.c
    )
    target_include_directories(module_iface_test PRIVATE
        ${WIN32_INC} ${ORIG_SRC}/portab ${ORIG_SRC}/ngircd)
    target_link_libraries(module_iface_test PRIVATE ws2_32)
    add_test(NAME module_iface_test COMMAND module_iface_test)
    if(MSVC)
        target_compile_options(module_iface_test PRIVATE /W3 /Zi /wd4005)
        target_link_options(module_iface_test PRIVATE /DEBUG)
    endif()

    add_executable(conf_whitebox_test
        tests/conf_whitebox_test.c
        tests/test_support.c
        ${ORIG_SRC}/ngircd/core/array.c
        ${ORIG_SRC}/ngircd/core/conf.c
        ${ORIG_SRC}/ngircd/core/module_iface.c
    )
    target_include_directories(conf_whitebox_test PRIVATE
        ${WIN32_INC} ${ORIG_SRC}/portab ${ORIG_SRC}/tool
        ${ORIG_SRC}/ipaddr ${ORIG_SRC}/ngircd)
    target_compile_definitions(conf_whitebox_test PRIVATE NGIRCD_UNITTEST)
    target_compile_options(conf_whitebox_test PRIVATE
        /W3 /Zi /wd4005 /wd4013 /wd4996 /wd4267 /wd4244 /wd4018 /wd4133 /wd4090)
    target_link_libraries(conf_whitebox_test PRIVATE ngportab ngtool ngipaddr ws2_32)
    target_link_options(conf_whitebox_test PRIVATE /DEBUG)
    add_test(NAME conf_whitebox_test COMMAND conf_whitebox_test)

    add_executable(log_io_resolver_test
        tests/log_io_resolver_test.c
        tests/log_io_test_support.c
        ${ORIG_SRC}/ngircd/core/array.c
        ${ORIG_SRC}/ngircd/core/log.c
        ${ORIG_SRC}/ngircd/net/io.c
        ${ORIG_SRC}/ngircd/core/module_iface.c
        ${WIN32_SRC}/win32_resolve.c
    )
    target_include_directories(log_io_resolver_test PRIVATE
        ${WIN32_INC} ${ORIG_SRC}/portab ${ORIG_SRC}/tool
        ${ORIG_SRC}/ipaddr ${ORIG_SRC}/ngircd)
    target_compile_definitions(log_io_resolver_test PRIVATE NGIRCD_UNITTEST)
    target_compile_options(log_io_resolver_test PRIVATE
        /W3 /Zi /wd4005 /wd4013 /wd4996 /wd4267 /wd4244 /wd4018 /wd4133 /wd4090)
    target_link_libraries(log_io_resolver_test PRIVATE ngportab ngtool ngipaddr ws2_32)
    target_link_options(log_io_resolver_test PRIVATE /DEBUG)
    add_test(NAME log_io_resolver_test COMMAND log_io_resolver_test)

    add_executable(login_state_test
        tests/login_state_test.c
        ${ORIG_SRC}/ngircd/proto/login_state.c)
    target_include_directories(login_state_test PRIVATE
        ${WIN32_INC} ${ORIG_SRC}/portab ${ORIG_SRC}/ngircd ${ORIG_SRC}/ngircd/proto)
    add_test(NAME login_state_test COMMAND login_state_test)

    add_executable(login_check_test
        tests/login_check_test.c
        ${ORIG_SRC}/ngircd/proto/login_check.c)
    target_include_directories(login_check_test PRIVATE
        ${WIN32_INC} ${ORIG_SRC}/portab ${ORIG_SRC}/ngircd ${ORIG_SRC}/ngircd/proto)
    add_test(NAME login_check_test COMMAND login_check_test)

    add_executable(join_check_test
        tests/join_check_test.c
        ${ORIG_SRC}/ngircd/proto/join_check.c)
    target_include_directories(join_check_test PRIVATE
        ${WIN32_INC} ${ORIG_SRC}/portab ${ORIG_SRC}/ngircd ${ORIG_SRC}/ngircd/proto)
    add_test(NAME join_check_test COMMAND join_check_test)

    add_executable(join_error_test
        tests/join_error_test.c
        ${ORIG_SRC}/ngircd/proto/join_error.c
        ${ORIG_SRC}/ngircd/proto/join_check.c)
    target_include_directories(join_error_test PRIVATE
        ${WIN32_INC} ${ORIG_SRC}/portab ${ORIG_SRC}/ngircd ${ORIG_SRC}/ngircd/proto)
    add_test(NAME join_error_test COMMAND join_error_test)

    add_executable(login_flow_test
        tests/login_flow_test.c
        ${ORIG_SRC}/ngircd/proto/login_flow.c)
    target_include_directories(login_flow_test PRIVATE
        ${WIN32_INC} ${ORIG_SRC}/portab ${ORIG_SRC}/ngircd ${ORIG_SRC}/ngircd/proto)
    add_test(NAME login_flow_test COMMAND login_flow_test)

    add_executable(irc_cap_end_test
        tests/irc_cap_end_test.c
        ${ORIG_SRC}/ngircd/proto/irc-cap.c)
    target_include_directories(irc_cap_end_test PRIVATE
        ${WIN32_INC} ${ORIG_SRC}/portab ${ORIG_SRC}/ngircd ${ORIG_SRC}/ngircd/proto)
    add_test(NAME irc_cap_end_test COMMAND irc_cap_end_test)

    add_executable(nick_check_test
        tests/nick_check_test.c
        ${ORIG_SRC}/ngircd/proto/nick_check.c)
    target_include_directories(nick_check_test PRIVATE
        ${WIN32_INC} ${ORIG_SRC}/portab ${ORIG_SRC}/ngircd ${ORIG_SRC}/ngircd/proto)
    add_test(NAME nick_check_test COMMAND nick_check_test)

    add_executable(user_check_test
        tests/user_check_test.c
        ${ORIG_SRC}/ngircd/proto/user_check.c)
    target_include_directories(user_check_test PRIVATE
        ${WIN32_INC} ${ORIG_SRC}/portab ${ORIG_SRC}/ngircd ${ORIG_SRC}/ngircd/proto)
    add_test(NAME user_check_test COMMAND user_check_test)

    add_executable(branch_coverage_test
        tests/branch_coverage_test.c
        ${ORIG_SRC}/ngircd/proto/login_check.c
        ${ORIG_SRC}/ngircd/proto/join_check.c
        ${ORIG_SRC}/ngircd/proto/join_error.c
        ${ORIG_SRC}/ngircd/proto/login_flow.c
        ${ORIG_SRC}/ngircd/proto/nick_check.c
        ${ORIG_SRC}/ngircd/proto/user_check.c
        ${ORIG_SRC}/ngircd/proto/message_check.c)
    target_include_directories(branch_coverage_test PRIVATE
        ${WIN32_INC} ${ORIG_SRC}/portab ${ORIG_SRC}/ngircd ${ORIG_SRC}/ngircd/proto)
    add_test(NAME branch_coverage_test COMMAND branch_coverage_test)

    add_executable(message_check_test
        tests/message_check_test.c
        ${ORIG_SRC}/ngircd/proto/message_check.c)
    target_include_directories(message_check_test PRIVATE
        ${WIN32_INC} ${ORIG_SRC}/portab ${ORIG_SRC}/ngircd ${ORIG_SRC}/ngircd/proto)
    add_test(NAME message_check_test COMMAND message_check_test)

    add_executable(full_coverage_test
        tests/full_coverage_test.c
        ${ORIG_SRC}/ngircd/proto/login_check.c
        ${ORIG_SRC}/ngircd/proto/join_check.c
        ${ORIG_SRC}/ngircd/proto/join_error.c
        ${ORIG_SRC}/ngircd/proto/login_flow.c
        ${ORIG_SRC}/ngircd/proto/login_state.c
        ${ORIG_SRC}/ngircd/proto/nick_check.c
        ${ORIG_SRC}/ngircd/proto/user_check.c
        ${ORIG_SRC}/ngircd/proto/message_check.c
        ${ORIG_SRC}/ngircd/core/module_iface.c)
    target_include_directories(full_coverage_test PRIVATE
        ${WIN32_INC} ${ORIG_SRC}/portab ${ORIG_SRC}/ngircd ${ORIG_SRC}/ngircd/proto)
    target_compile_options(full_coverage_test PRIVATE /W3 /wd4005)
    target_link_libraries(full_coverage_test PRIVATE ws2_32)
    add_test(NAME full_coverage_test COMMAND full_coverage_test)

    add_test(NAME ngircd_configtest
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/ngircd.exe
                --config ${CMAKE_SOURCE_DIR}/build/ngircd-win32-test.conf --configtest
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    add_test(NAME ngircd_configtest_rich
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/ngircd.exe
                --config ${CMAKE_SOURCE_DIR}/tests/configs/rich.conf --configtest
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    add_test(NAME ngircd_configtest_ipv6
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/ngircd.exe
                --config ${CMAKE_SOURCE_DIR}/tests/configs/ipv6.conf --configtest
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    add_test(NAME ngircd_configtest_unknown_section
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/ngircd.exe
                --config ${CMAKE_SOURCE_DIR}/tests/configs/unknown_section.conf --configtest
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    add_test(NAME ngircd_configtest_multi_server
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/ngircd.exe
                --config ${CMAKE_SOURCE_DIR}/tests/configs/multi_server.conf --configtest
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    add_test(NAME ngircd_configtest_long_values
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/ngircd.exe
                --config ${CMAKE_SOURCE_DIR}/tests/configs/long_values.conf --configtest
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    add_test(NAME ngircd_configtest_invalid
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/ngircd.exe
                --config ${CMAKE_SOURCE_DIR}/tests/configs/does-not-exist.conf --configtest
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    set_tests_properties(ngircd_configtest_invalid PROPERTIES WILL_FAIL TRUE)

    # ---------------------------------------------------------------------------
    # New unit tests: module API layer and host wiring
    # ---------------------------------------------------------------------------

    # core_runtime_test: version, status helpers, module_kind constants
    add_executable(core_runtime_test
        tests/core_runtime_test.c
        ${SHARED_SRC}/core_runtime/core_runtime.c
    )
    target_include_directories(core_runtime_test PRIVATE ${SHARED_INC})
    target_compile_definitions(core_runtime_test PRIVATE
        CORE_RUNTIME_BUILD _CRT_SECURE_NO_WARNINGS)
    if(MSVC)
        target_compile_options(core_runtime_test PRIVATE /W3 /Zi /wd4005)
        target_link_options(core_runtime_test PRIVATE /DEBUG)
    endif()
    add_test(NAME core_runtime_test COMMAND core_runtime_test)

    # host_wiring_test: host_get_wiring_v1, wire() validation with mock structs
    add_executable(host_wiring_test
        tests/host_wiring_test.c
        ${SHARED_SRC}/host/host.c
    )
    target_include_directories(host_wiring_test PRIVATE
        ${SHARED_INC} ${WIN32_INC})
    target_compile_definitions(host_wiring_test PRIVATE
        HOST_BUILD _CRT_SECURE_NO_WARNINGS _WINSOCK_DEPRECATED_NO_WARNINGS)
    target_link_libraries(host_wiring_test PRIVATE core_runtime)
    if(MSVC)
        target_compile_options(host_wiring_test PRIVATE
            /W3 /Zi /wd4005 /wd4244 /wd4090)
        target_link_options(host_wiring_test PRIVATE /DEBUG)
    endif()
    add_test(NAME host_wiring_test COMMAND host_wiring_test)

    # logging_module_test: logging_get_api_v1, module_get_api_v1, smoke
    add_executable(logging_module_test
        tests/logging_module_test.c
        ${SHARED_SRC}/logging/logging.c
    )
    target_include_directories(logging_module_test PRIVATE
        ${SHARED_INC} ${WIN32_INC}
        ${ORIG_SRC} ${ORIG_SRC}/portab ${ORIG_SRC}/ngircd)
    target_compile_definitions(logging_module_test PRIVATE
        LOGGING_BUILD _CRT_SECURE_NO_WARNINGS _WINSOCK_DEPRECATED_NO_WARNINGS)
    target_link_libraries(logging_module_test PRIVATE core_runtime)
    if(MSVC)
        target_compile_options(logging_module_test PRIVATE
            /W3 /Zi /wd4005 /wd4013 /wd4996)
        target_link_options(logging_module_test PRIVATE /DEBUG)
    endif()
    add_test(NAME logging_module_test COMMAND logging_module_test)

    # config_module_test: config_get_api_v1, get_bootstrap, module_get_api_v1
    add_executable(config_module_test
        tests/config_module_test.c
        tests/test_support.c
        tests/conf_func_stubs.c
        ${ORIG_SRC}/ngircd/core/array.c
        ${SHARED_SRC}/config/config.c
    )
    target_include_directories(config_module_test PRIVATE
        ${SHARED_INC} ${WIN32_INC}
        ${ORIG_SRC} ${ORIG_SRC}/portab ${ORIG_SRC}/tool ${ORIG_SRC}/ipaddr ${ORIG_SRC}/ngircd)
    target_compile_definitions(config_module_test PRIVATE
        CONFIG_BUILD _CRT_SECURE_NO_WARNINGS _WINSOCK_DEPRECATED_NO_WARNINGS)
    target_link_libraries(config_module_test PRIVATE core_runtime ngportab ws2_32)
    if(MSVC)
        target_compile_options(config_module_test PRIVATE
            /W3 /Zi /wd4005 /wd4013 /wd4996 /wd4267 /wd4244 /wd4018 /wd4133 /wd4090)
        target_link_options(config_module_test PRIVATE /DEBUG)
    endif()
    add_test(NAME config_module_test COMMAND config_module_test)

    # platform_module_test: platform_get_api_v1, module_get_api_v1, error paths
    add_executable(platform_module_test
        tests/platform_module_test.c
        ${SHARED_SRC}/platform/platform.c
    )
    target_include_directories(platform_module_test PRIVATE
        ${SHARED_INC} ${WIN32_INC})
    target_compile_definitions(platform_module_test PRIVATE
        PLATFORM_BUILD _CRT_SECURE_NO_WARNINGS _WINSOCK_DEPRECATED_NO_WARNINGS)
    target_link_libraries(platform_module_test PRIVATE core_runtime)
    if(MSVC)
        target_compile_options(platform_module_test PRIVATE
            /W3 /Zi /wd4005 /wd4996)
        target_link_options(platform_module_test PRIVATE /DEBUG)
    endif()
    add_test(NAME platform_module_test COMMAND platform_module_test)

    # server_app_unit_test: links DLLs; tests create/start/stop/build_proto_id
    # and module_get_api_v1 via GetProcAddress
    add_executable(server_app_unit_test
        tests/server_app_unit_test.c
    )
    target_include_directories(server_app_unit_test PRIVATE
        ${SHARED_INC} ${WIN32_INC} ${ORIG_SRC}/portab)
    target_compile_definitions(server_app_unit_test PRIVATE
        _CRT_SECURE_NO_WARNINGS _WINSOCK_DEPRECATED_NO_WARNINGS)
    target_link_libraries(server_app_unit_test PRIVATE
        server_app logging config net_transport core_runtime)
    if(MSVC)
        target_compile_options(server_app_unit_test PRIVATE
            /W3 /Zi /wd4005 /wd4996)
        target_link_options(server_app_unit_test PRIVATE /DEBUG)
    endif()
    add_test(NAME server_app_unit_test COMMAND server_app_unit_test)

endif()
