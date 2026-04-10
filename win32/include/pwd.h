/* Win32 stub for <pwd.h> */
#ifndef _WIN32_PWD_H
#define _WIN32_PWD_H

struct passwd {
	char *pw_name;
	char *pw_passwd;
	int pw_uid;
	int pw_gid;
	char *pw_dir;
	char *pw_shell;
};

static __inline struct passwd *getpwnam(const char *name) { (void)name; return NULL; }
static __inline struct passwd *getpwuid(int uid) { (void)uid; return NULL; }
static __inline void endpwent(void) {}

#endif
