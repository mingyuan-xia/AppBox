/*
 * binder_helper.c
 * implementation of binder_helper.h
 */

#include <stdio.h>
#include <linux/binder.h>
#include <string.h>
#include "uchar.h"
#include "binder.h"
#include "ptraceaux.h"
#include "config.h"
#include "intentfw.h"
#include "log.h"

static void _do_write_read(pid_t pid, struct binder_write_read *u)
{
	long const write_size = u->write_size;
	uint8_t write_buf[write_size];
	uint8_t *cur = write_buf;
	uint32_t cmd = 0;
	ptrace_read_data(pid, write_buf, (tracee_ptr_t) u->write_buffer, write_size);
	while ((long)(cur - write_buf) < write_size) {
		cmd = *(uint32_t *)cur;
		cur += sizeof(uint32_t);
		switch (cmd) {
			case BC_TRANSACTION:
				handle_intent(pid, (struct binder_transaction_data *)cur);
				cur += sizeof (struct binder_transaction_data);
				break;
			case BC_REPLY:
				cur += sizeof(struct binder_transaction_data);
				break;
			case BC_ACQUIRE_RESULT:
				cur += sizeof(int);
				break;
			case BC_FREE_BUFFER:
				cur += sizeof(int);
				break;
			case BC_INCREFS:
			case BC_ACQUIRE:
			case BC_RELEASE:
			case BC_DECREFS:
				cur += sizeof(int);
				break;
			case BC_INCREFS_DONE:
			case BC_ACQUIRE_DONE:
			case BC_ATTEMPT_ACQUIRE:
				cur += sizeof (struct binder_ptr_cookie);
				break;
			case BC_REGISTER_LOOPER:
			case BC_ENTER_LOOPER:
			case BC_EXIT_LOOPER:
				break;
			case BC_REQUEST_DEATH_NOTIFICATION:
			case BC_CLEAR_DEATH_NOTIFICATION:
				cur += sizeof(struct binder_ptr_cookie);
				break;
			case BC_DEAD_BINDER_DONE:
				cur += sizeof(void*);
				break;
			default:
				LOGD("[%d] unsupported binder write cmd %d\n", pid, cmd);
				break;

		}
	}
	
}

void handle_binder(sandbox_t *sandbox, pid_t pid)
{
	long cmd = ptrace_get_syscall_arg(pid, 1);
	switch (cmd) {
		case BINDER_WRITE_READ: {
			LOGD("binder call: BINDER_WRITE_READ\n");
			struct binder_write_read u;
			tracee_ptr_t ptr = (tracee_ptr_t) ptrace_get_syscall_arg(pid, 2);
			ptrace_read_data(pid, &u, ptr, sizeof(u));
			_do_write_read(pid, &u);
			break;
		}
		case BINDER_SET_IDLE_TIMEOUT:
			LOGD("binder call: BINDER_SET_IDLE_TIMEOUT\n");
			break;
		case BINDER_SET_MAX_THREADS:
			LOGD("binder call: BINDER_SET_MAX_THREADS\n");
			break;
		case BINDER_SET_IDLE_PRIORITY:
			LOGD("binder call: BINDER_SET_IDLE_PRIORITY\n");
			break;
		case BINDER_SET_CONTEXT_MGR:
			LOGD("binder call: BINDER_SET_CONTEXT_MGR\n");
			break;
		case BINDER_THREAD_EXIT:
			LOGD("binder call: BINDER_THREAD_EXIT\n");
			break;
		case BINDER_VERSION: {
			LOGD("binder call: BINDER_VERSION\n");
			struct binder_version u;
			break;
		}
		default:
			LOGD("binder call: unknown\n");
			break;
	}
}

