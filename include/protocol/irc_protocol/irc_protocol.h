#ifndef IRC_PROTOCOL_IRC_PROTOCOL_H
#define IRC_PROTOCOL_IRC_PROTOCOL_H

#include <stdint.h>

#include "core/runtime/core_runtime.h"

#ifdef _WIN32
#  ifdef IRC_PROTOCOL_BUILD
#    define IRC_PROTOCOL_API __declspec(dllexport)
#  else
#    define IRC_PROTOCOL_API __declspec(dllimport)
#  endif
#  define IRC_PROTOCOL_CALL __cdecl
#else
#  define IRC_PROTOCOL_API
#  define IRC_PROTOCOL_CALL
#endif

typedef struct irc_protocol_api {
    uint32_t api_major;
    uint32_t api_minor;
    core_status_t (IRC_PROTOCOL_CALL *parse_line)(const char *line, size_t line_size);
    core_status_t (IRC_PROTOCOL_CALL *format_reply)(uint16_t numeric, const char *text);
} irc_protocol_api_t;

typedef const irc_protocol_api_t * (IRC_PROTOCOL_CALL *irc_protocol_get_api_v1_fn)(void);

IRC_PROTOCOL_API const irc_protocol_api_t * IRC_PROTOCOL_CALL irc_protocol_get_api_v1(void);

#endif

