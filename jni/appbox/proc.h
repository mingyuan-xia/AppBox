#ifndef PROC_H_
#define PROC_H_

#include <unistd.h>
#include <stddef.h>

#define DEV_BINDER "/dev/binder"

extern int find_fd(pid_t pid, char const *fname);
extern size_t get_tids(pid_t **const listptr, size_t *const sizeptr, const pid_t pid);

#endif /* PROC_H_ */
