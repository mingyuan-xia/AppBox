#include "config.h"
#include "zygote_helper.h"
#include "ptraceaux.h"
#include "sandbox.h"
#include "log.h"
#include "proc.h"
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <signal.h>
#include <string.h>

int main(int argc, char *argv[])
{
	LOGD("Usage: sandbox [pid]");
	if (create_link(SANDBOX_STORAGE_PATH, SANDBOX_LINK) < 0) {
		LOGD("sandbox fake root existed\n");
	}
	pid_t pid = -1;
	if (argc >= 2) {
		pid = atoi(argv[1]);
		int opt = PTRACE_O_TRACEFORK | PTRACE_O_TRACECLONE | PTRACE_O_TRACESYSGOOD;
		pid_t *tid;
		size_t tid_max = 0;
		size_t tids = get_tids(&tid, &tid_max, pid);
		size_t i;
		LOGD("number of tids=%d\n", tids);
		for (i = 0; i < tids; ++i) {
			if (ptrace_attach(tid[i]) == -1) {
				LOGD("failed to attach to task of target process pid=%d tid=%d\n", pid, tid[i]);
				return 1;
			}
			ptrace_setopt(tid[i], opt);
			ptrace_cont(tid[i]);
			LOGD("attached to tid=%d\n", tid[i]);
		}
		free(tid);
	} else {
		LOGD("finding zygote\n");
		pid_t zygote_pid = zygote_find_process();
		LOGD("monitor zygote pid=%d\n", zygote_pid);
		pid = ptrace_zygote(zygote_pid);
	}
	if (pid > 0) {
		sandbox_t sandbox;
		sandbox_init(&sandbox, pid, 0);
		sandbox.binder_fd = find_fd(pid, DEV_BINDER);
		LOGD("sandboxing initialized pid=%d\n", pid);
		LOGD("starting sandbox pid=%d\n", pid);
		sandbox_start(&sandbox);
	}
	return 0;
}

