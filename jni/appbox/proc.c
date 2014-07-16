#include "proc.h"
#include <stdio.h>
#include <fnmatch.h>
#include <errno.h>
#include <dirent.h>

int find_fd(pid_t pid, char const *fname)
{
	char path[64], buf[256];
	int len, fd;
	for (fd = 0; fd < 32; ++fd) {
		snprintf(path, sizeof(path), "/proc/%d/fd/%d", pid, fd);
		len = readlink(path, buf, sizeof(buf) - 1);
		if (len <= 0)
			continue;
		buf[len] = '\0';
		if (fnmatch(buf, fname, FNM_PATHNAME) == 0)
			return fd;
	}
	return -1;
}

/* Similar to getline(), except gets process pid task IDs.
 * Returns positive (number of TIDs in list) if success,
 * otherwise 0 with errno set. */
size_t get_tids(pid_t **const listptr, size_t *const sizeptr, const pid_t pid)
{
    char     dirname[64];
    DIR     *dir;
    pid_t   *list;
    size_t   size, used = 0;

    if (!listptr || !sizeptr || pid < (pid_t)1) {
        errno = EINVAL;
        return (size_t)0;
    }

    if (*sizeptr > 0) {
        list = *listptr;
        size = *sizeptr;
    } else {
        list = *listptr = NULL;
        size = *sizeptr = 0;
    }

    if (snprintf(dirname, sizeof dirname, "/proc/%d/task/", (int)pid) >= (int)sizeof dirname) {
        errno = ENOTSUP;
        return (size_t)0;
    }

    dir = opendir(dirname);
    if (!dir) {
        errno = ESRCH;
        return (size_t)0;
    }

    while (1) {
        struct dirent *ent;
        int            value;
        char           dummy;

        errno = 0;
        ent = readdir(dir);
        if (!ent)
            break;

        /* Parse TIDs. Ignore non-numeric entries. */
        if (sscanf(ent->d_name, "%d%c", &value, &dummy) != 1)
            continue;

        /* Ignore obviously invalid entries. */
        if (value < 1)
            continue;

        /* Make sure there is room for another TID. */
        if (used >= size) {
            size = (used | 127) + 128;
            list = realloc(list, size * sizeof list[0]);
            if (!list) {
                closedir(dir);
                errno = ENOMEM;
                return (size_t)0;
            }
            *listptr = list;
            *sizeptr = size;
        }

        /* Add to list. */
        list[used++] = (pid_t)value;
    }
    if (errno) {
        const int saved_errno = errno;
        closedir(dir);
        errno = saved_errno;
        return (size_t)0;
    }
    if (closedir(dir)) {
        errno = EIO;
        return (size_t)0;
    }

    /* None? */
    if (used < 1) {
        errno = ESRCH;
        return (size_t)0;
    }

    /* Make sure there is room for a terminating (pid_t)0. */
    if (used >= size) {
        size = used + 1;
        list = realloc(list, size * sizeof list[0]);
        if (!list) {
            errno = ENOMEM;
            return (size_t)0;
        }
        *listptr = list;
        *sizeptr = size;
    }

    /* Terminate list; done. */
    list[used] = (pid_t)0;
    errno = 0;
    return used;
}
