/* Win32 stub for <grp.h> */
#ifndef _WIN32_GRP_H
#define _WIN32_GRP_H

struct group {
	char *gr_name;
	int gr_gid;
};

static __inline struct group *getgrgid(int gid) { (void)gid; return NULL; }
static __inline struct group *getgrnam(const char *name) { (void)name; return NULL; }

#endif
