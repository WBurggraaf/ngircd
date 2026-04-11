/* Win32 minimal implementation of <dirent.h> */
#ifndef _WIN32_DIRENT_H
#define _WIN32_DIRENT_H

#include <string.h>
#include <stdlib.h>

struct dirent {
	char d_name[MAX_PATH];
};

typedef struct {
	HANDLE hFind;
	WIN32_FIND_DATAA ffd;
	struct dirent entry;
	int first;
	char path[MAX_PATH];
} DIR;

static __inline DIR *opendir(const char *name) {
	DIR *d = (DIR *)malloc(sizeof(DIR));
	if (!d) return NULL;
	snprintf(d->path, MAX_PATH, "%s\\*", name);
	d->hFind = FindFirstFileA(d->path, &d->ffd);
	if (d->hFind == INVALID_HANDLE_VALUE) {
		free(d);
		return NULL;
	}
	d->first = 1;
	return d;
}

static __inline struct dirent *readdir(DIR *d) {
	if (!d) return NULL;
	if (d->first) {
		d->first = 0;
	} else {
		if (!FindNextFileA(d->hFind, &d->ffd))
			return NULL;
	}
	strncpy(d->entry.d_name, d->ffd.cFileName, MAX_PATH - 1);
	d->entry.d_name[MAX_PATH - 1] = '\0';
	return &d->entry;
}

static __inline int closedir(DIR *d) {
	if (!d) return -1;
	FindClose(d->hFind);
	free(d);
	return 0;
}

#endif
