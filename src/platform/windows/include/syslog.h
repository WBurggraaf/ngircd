/* Win32 stub for <syslog.h> */
#ifndef _WIN32_SYSLOG_H
#define _WIN32_SYSLOG_H

#define LOG_EMERG   0
#define LOG_ALERT   1
#define LOG_CRIT    2
#define LOG_ERR     3
#define LOG_WARNING 4
#define LOG_NOTICE  5
#define LOG_INFO    6
#define LOG_DEBUG   7

#define LOG_DAEMON  0
#define LOG_USER    0
#define LOG_LOCAL0  0
#define LOG_LOCAL1  0
#define LOG_LOCAL2  0
#define LOG_LOCAL3  0
#define LOG_LOCAL4  0
#define LOG_LOCAL5  0
#define LOG_LOCAL6  0
#define LOG_LOCAL7  0

static __inline void openlog(const char *ident, int option, int facility) {
	(void)ident; (void)option; (void)facility;
}
static __inline void syslog(int priority, const char *format, ...) {
	(void)priority; (void)format;
}
static __inline void closelog(void) {}

#endif
