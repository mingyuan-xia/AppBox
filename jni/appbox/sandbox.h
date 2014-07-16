/*
 * process_helper.h
 * The helper functions for communicating with the app process
 */

#ifndef SANDBOX_H_
#define SANDBOX_H_

#include "sys/types.h"

#define SANDBOX_CHROOT_PRIVATE_FOLDER   (0x1 << 0)
#define SANDBOX_CHROOT_EXTERNAL_STORAGE (0x1 << 1)
#define SANDBOX_INTENT_FIREWALL         (0x1 << 2)
#define SANDBOX_NET_FIREWALL            (0x1 << 3)

typedef struct sandbox_s {
	/* the flag */
	int flag;
	/* the binder fd of the app process */
	int binder_fd;
	/* the uid and gid */
	uid_t uid;
	gid_t gid;
	/* hooks */
	struct {
		void (*before_handle_syscall)(struct sandbox_s *sandbox, pid_t pid, long syscall_nr);
		void (*after_handle_syscall)(struct sandbox_s *sandbox, pid_t pid, long syscall_nr);
	}hooks;
}sandbox_t;

/**
 * initialize a sandbox for a particular app process
 */
extern sandbox_t *sandbox_init(sandbox_t *sandbox, int flag);

extern void sandbox_handle_syscall(sandbox_t *sandbox, long scno, pid_t pid);

#endif /* SANDBOX_H_ */

