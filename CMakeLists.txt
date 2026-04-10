cmake_minimum_required(VERSION 3.15)
project(ngircd C)

set(CMAKE_C_STANDARD 99)
include(CTest)

# Source directories
set(ORIG_SRC "${CMAKE_SOURCE_DIR}/original/src")
set(WIN32_SRC "${CMAKE_SOURCE_DIR}/win32/src")
set(WIN32_INC "${CMAKE_SOURCE_DIR}/win32/include")

# --- Portability library (libngportab) ---
add_library(ngportab STATIC
    ${ORIG_SRC}/portab/strdup.c
    ${ORIG_SRC}/portab/strlcpy.c
    ${ORIG_SRC}/portab/strndup.c
    ${ORIG_SRC}/portab/strtok_r.c
    ${ORIG_SRC}/portab/vsnprintf.c
)
target_include_directories(ngportab PUBLIC
    ${WIN32_INC}          # config.h and compat headers (HIGHEST PRIORITY)
    ${ORIG_SRC}/portab
)
target_compile_definitions(ngportab PRIVATE _CRT_SECURE_NO_WARNINGS _WINSOCK_DEPRECATED_NO_WARNINGS)

# --- Tool library (libngtool) ---
add_library(ngtool STATIC
    ${ORIG_SRC}/tool/tool.c
)
target_include_directories(ngtool PUBLIC
    ${WIN32_INC}
    ${ORIG_SRC}/portab
    ${ORIG_SRC}/tool
)
target_compile_definitions(ngtool PRIVATE _CRT_SECURE_NO_WARNINGS _WINSOCK_DEPRECATED_NO_WARNINGS)

# --- IP Address library (libngipaddr) ---
add_library(ngipaddr STATIC
    ${ORIG_SRC}/ipaddr/ng_ipaddr.c
)
target_include_directories(ngipaddr PUBLIC
    ${WIN32_INC}
    ${ORIG_SRC}/portab
    ${ORIG_SRC}/ipaddr
)
target_compile_definitions(ngipaddr PRIVATE _CRT_SECURE_NO_WARNINGS _WINSOCK_DEPRECATED_NO_WARNINGS)
target_link_libraries(ngipaddr ws2_32)

# --- Main ngircd executable ---
set(NGIRCD_SOURCES
    ${ORIG_SRC}/ngircd/platform/ngircd.c
    ${ORIG_SRC}/ngircd/core/array.c
    ${ORIG_SRC}/ngircd/channel.c
    ${ORIG_SRC}/ngircd/core/class.c
    ${ORIG_SRC}/ngircd/core/client.c
    ${ORIG_SRC}/ngircd/core/client-cap.c
    ${ORIG_SRC}/ngircd/core/conf.c
    ${ORIG_SRC}/ngircd/net/conn.c
    ${ORIG_SRC}/ngircd/net/conn-encoding.c
    ${ORIG_SRC}/ngircd/net/conn-func.c
    ${ORIG_SRC}/ngircd/core/hash.c
    ${ORIG_SRC}/ngircd/net/io.c
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
    ${WIN32_SRC}/win32_main.c
)

add_executable(ngircd ${NGIRCD_SOURCES})

# Include directories: win32 compat headers must come FIRST
target_include_directories(ngircd PRIVATE
    ${WIN32_INC}              # Highest priority: our config.h and compat headers
    ${ORIG_SRC}/portab
    ${ORIG_SRC}/tool
    ${ORIG_SRC}/ipaddr
    ${ORIG_SRC}/ngircd
)

target_compile_definitions(ngircd PRIVATE
    _CRT_SECURE_NO_WARNINGS
    _WINSOCK_DEPRECATED_NO_WARNINGS
)

# Rename main() only for the original ngircd.c, not for win32_main.c
set_source_files_properties(${ORIG_SRC}/ngircd/platform/ngircd.c PROPERTIES
    COMPILE_DEFINITIONS "main=ngircd_main"
)

target_link_libraries(ngircd
    ngportab
    ngtool
    ngipaddr
    ws2_32
)

if(BUILD_TESTING)
    add_executable(module_iface_test
        tests/module_iface_test.c
        ${ORIG_SRC}/ngircd/core/module_iface.c
    )
    target_include_directories(module_iface_test PRIVATE
        ${WIN32_INC}
        ${ORIG_SRC}/portab
        ${ORIG_SRC}/ngircd
    )
    target_link_libraries(module_iface_test PRIVATE ws2_32)
    add_test(NAME module_iface_test COMMAND module_iface_test)

    add_executable(conf_whitebox_test
        tests/conf_whitebox_test.c
        tests/test_support.c
        ${ORIG_SRC}/ngircd/core/array.c
        ${ORIG_SRC}/ngircd/core/conf.c
        ${ORIG_SRC}/ngircd/core/module_iface.c
    )
    target_include_directories(conf_whitebox_test PRIVATE
        ${WIN32_INC}
        ${ORIG_SRC}/portab
        ${ORIG_SRC}/tool
        ${ORIG_SRC}/ipaddr
        ${ORIG_SRC}/ngircd
    )
    target_compile_definitions(conf_whitebox_test PRIVATE NGIRCD_UNITTEST)
    target_compile_options(conf_whitebox_test PRIVATE /W3 /Zi /wd4005 /wd4013 /wd4996 /wd4267 /wd4244 /wd4018 /wd4133 /wd4090)
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

    add_executable(login_state_test
        tests/login_state_test.c
        ${ORIG_SRC}/ngircd/proto/login_state.c
    )
    target_include_directories(login_state_test PRIVATE
        ${WIN32_INC}
        ${ORIG_SRC}/portab
        ${ORIG_SRC}/ngircd
        ${ORIG_SRC}/ngircd/proto
    )
    add_test(NAME login_state_test COMMAND login_state_test)

    add_executable(login_check_test
        tests/login_check_test.c
        ${ORIG_SRC}/ngircd/proto/login_check.c
    )
    target_include_directories(login_check_test PRIVATE
        ${WIN32_INC}
        ${ORIG_SRC}/portab
        ${ORIG_SRC}/ngircd
        ${ORIG_SRC}/ngircd/proto
    )
    add_test(NAME login_check_test COMMAND login_check_test)

    add_executable(join_check_test
        tests/join_check_test.c
        ${ORIG_SRC}/ngircd/proto/join_check.c
    )
    target_include_directories(join_check_test PRIVATE
        ${WIN32_INC}
        ${ORIG_SRC}/portab
        ${ORIG_SRC}/ngircd
        ${ORIG_SRC}/ngircd/proto
    )
    add_test(NAME join_check_test COMMAND join_check_test)

    add_executable(join_error_test
        tests/join_error_test.c
        ${ORIG_SRC}/ngircd/proto/join_error.c
        ${ORIG_SRC}/ngircd/proto/join_check.c
    )
    target_include_directories(join_error_test PRIVATE
        ${WIN32_INC}
        ${ORIG_SRC}/portab
        ${ORIG_SRC}/ngircd
        ${ORIG_SRC}/ngircd/proto
    )
    add_test(NAME join_error_test COMMAND join_error_test)

    add_executable(login_flow_test
        tests/login_flow_test.c
        ${ORIG_SRC}/ngircd/proto/login_flow.c
    )
    target_include_directories(login_flow_test PRIVATE
        ${WIN32_INC}
        ${ORIG_SRC}/portab
        ${ORIG_SRC}/ngircd
        ${ORIG_SRC}/ngircd/proto
    )
    add_test(NAME login_flow_test COMMAND login_flow_test)

    add_executable(irc_cap_end_test
        tests/irc_cap_end_test.c
        ${ORIG_SRC}/ngircd/proto/irc-cap.c
    )
    target_include_directories(irc_cap_end_test PRIVATE
        ${WIN32_INC}
        ${ORIG_SRC}/portab
        ${ORIG_SRC}/ngircd
        ${ORIG_SRC}/ngircd/proto
    )
    add_test(NAME irc_cap_end_test COMMAND irc_cap_end_test)

    add_executable(nick_check_test
        tests/nick_check_test.c
        ${ORIG_SRC}/ngircd/proto/nick_check.c
    )
    target_include_directories(nick_check_test PRIVATE
        ${WIN32_INC}
        ${ORIG_SRC}/portab
        ${ORIG_SRC}/ngircd
        ${ORIG_SRC}/ngircd/proto
    )
    add_test(NAME nick_check_test COMMAND nick_check_test)

    add_executable(user_check_test
        tests/user_check_test.c
        ${ORIG_SRC}/ngircd/proto/user_check.c
    )
    target_include_directories(user_check_test PRIVATE
        ${WIN32_INC}
        ${ORIG_SRC}/portab
        ${ORIG_SRC}/ngircd
        ${ORIG_SRC}/ngircd/proto
    )
    add_test(NAME user_check_test COMMAND user_check_test)

    add_executable(branch_coverage_test
        tests/branch_coverage_test.c
        ${ORIG_SRC}/ngircd/proto/login_check.c
        ${ORIG_SRC}/ngircd/proto/join_check.c
        ${ORIG_SRC}/ngircd/proto/join_error.c
        ${ORIG_SRC}/ngircd/proto/login_flow.c
        ${ORIG_SRC}/ngircd/proto/nick_check.c
        ${ORIG_SRC}/ngircd/proto/user_check.c
        ${ORIG_SRC}/ngircd/proto/message_check.c
    )
    target_include_directories(branch_coverage_test PRIVATE
        ${WIN32_INC}
        ${ORIG_SRC}/portab
        ${ORIG_SRC}/ngircd
        ${ORIG_SRC}/ngircd/proto
    )
    add_test(NAME branch_coverage_test COMMAND branch_coverage_test)

    add_executable(message_check_test
        tests/message_check_test.c
        ${ORIG_SRC}/ngircd/proto/message_check.c
    )
    target_include_directories(message_check_test PRIVATE
        ${WIN32_INC}
        ${ORIG_SRC}/portab
        ${ORIG_SRC}/ngircd
        ${ORIG_SRC}/ngircd/proto
    )
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
        ${ORIG_SRC}/ngircd/core/module_iface.c
    )
    target_include_directories(full_coverage_test PRIVATE
        ${WIN32_INC}
        ${ORIG_SRC}/portab
        ${ORIG_SRC}/ngircd
        ${ORIG_SRC}/ngircd/proto
    )
    target_compile_options(full_coverage_test PRIVATE /W3 /wd4005)
    target_link_libraries(full_coverage_test PRIVATE ws2_32)
    add_test(NAME full_coverage_test COMMAND full_coverage_test)
    target_include_directories(log_io_resolver_test PRIVATE
        ${WIN32_INC}
        ${ORIG_SRC}/portab
        ${ORIG_SRC}/tool
        ${ORIG_SRC}/ipaddr
        ${ORIG_SRC}/ngircd
    )
    target_compile_definitions(log_io_resolver_test PRIVATE NGIRCD_UNITTEST)
    target_compile_options(log_io_resolver_test PRIVATE /W3 /Zi /wd4005 /wd4013 /wd4996 /wd4267 /wd4244 /wd4018 /wd4133 /wd4090)
    target_link_libraries(log_io_resolver_test PRIVATE ngportab ngtool ngipaddr ws2_32)
    target_link_options(log_io_resolver_test PRIVATE /DEBUG)
    add_test(NAME log_io_resolver_test COMMAND log_io_resolver_test)

    add_test(
        NAME ngircd_configtest
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/ngircd.exe --config ${CMAKE_SOURCE_DIR}/build/ngircd-win32-test.conf --configtest
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    add_test(
        NAME ngircd_configtest_rich
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/ngircd.exe --config ${CMAKE_SOURCE_DIR}/tests/configs/rich.conf --configtest
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    add_test(
        NAME ngircd_configtest_ipv6
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/ngircd.exe --config ${CMAKE_SOURCE_DIR}/tests/configs/ipv6.conf --configtest
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    add_test(
        NAME ngircd_configtest_unknown_section
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/ngircd.exe --config ${CMAKE_SOURCE_DIR}/tests/configs/unknown_section.conf --configtest
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    add_test(
        NAME ngircd_configtest_multi_server
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/ngircd.exe --config ${CMAKE_SOURCE_DIR}/tests/configs/multi_server.conf --configtest
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    add_test(
        NAME ngircd_configtest_long_values
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/ngircd.exe --config ${CMAKE_SOURCE_DIR}/tests/configs/long_values.conf --configtest
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    add_test(
        NAME ngircd_configtest_invalid
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/ngircd.exe --config ${CMAKE_SOURCE_DIR}/tests/configs/does-not-exist.conf --configtest
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(ngircd_configtest_invalid PROPERTIES WILL_FAIL TRUE)
endif()

# MSVC-specific settings
if(MSVC)
    # Disable specific warnings that are noise for this Unix-origin codebase
    target_compile_options(ngircd PRIVATE
        /W3
        /Zi
        /wd4005   # NDEBUG redefinition from portab.h
        /wd4013   # implicit declarations from Unix compatibility shims
        /wd4996   # deprecated POSIX names
        /wd4267   # size_t to int conversion
        /wd4244   # possible loss of data
        /wd4018   # signed/unsigned mismatch
        /wd4133   # incompatible types
        /wd4090   # different const qualifiers
    )
    target_compile_options(ngportab PRIVATE /W3 /Zi /wd4996 /wd4005)
    target_compile_options(ngtool PRIVATE /W3 /Zi /wd4996 /wd4005)
    target_compile_options(ngipaddr PRIVATE /W3 /Zi /wd4996 /wd4267 /wd4005)
    target_compile_options(module_iface_test PRIVATE /W3 /Zi /wd4005)
    target_link_options(ngircd PRIVATE /DEBUG)
    target_link_options(module_iface_test PRIVATE /DEBUG)
endif()
