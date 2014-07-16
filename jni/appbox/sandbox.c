#include <sys/syscall.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "sandbox.h"
#include "binder.h"
#include "ptraceaux.h"
#include "log.h"

#define DEV_BINDER "/dev/binder"
#define IS_WRITE(oflag) ((oflag & O_WRONLY) != 0 || (oflag & O_RDWR) != 0)

/**
 * Handle syscalls that take a path as the first parameter
 */
static void _handle_file_series_syscalls(pid_t pid, char* syscall, int flag, uid_t uid, gid_t gid)
{
	// TODO
	tracee_ptr_t path_ptr = (tracee_ptr_t) ptrace_get_syscall_arg(pid, 0);
	int len = ptrace_strlen(pid, path_ptr);
	char path[len + 1];
	ptrace_read_data(pid, path, path_ptr, len + 1);
#if 0
	int nth_dir;

	if ((flag & SANDBOX_CHROOT_PRIVATE_FOLDER) && (nth_dir = check_prefix_dir(path,SANDBOX_PATH_INTERNAL)) > 0) {
		//internal file storage sandbox
		char* sub_dir = get_nth_dir(path, nth_dir + 2);
		if (!check_prefix(sub_dir, SANDBOX_PATH_INTERNAL_EXCLUDE)) {
			char new_path[len + 1];
			//replace dir in path with LINK_PREFIX
			char* second_dir = get_nth_dir(path, nth_dir + 1);
			strcpy(new_path, SANDBOX_LINK);
			strcat(new_path, second_dir);
			ptrace_write_data(pid, new_path, path_ptr, len + 1);
			// create require folder
			create_nth_dir(new_path, 3, uid, gid, 0751);
			LOGD("pid %d %s: %s ==> new path: %s", pid, syscall, path, new_path);

			// return from open syscall, reset the path
			pid = waitpid(pid, NULL, __WALL);

			ptrace_write_data(pid, path, path_ptr, len + 1);
			long result = ptrace_get_syscall_arg(pid, 0);
			LOGD(" = %ld\n", result);

			return;
		}
	/* } else if ((flag & SANDBOX_FLAG) && FILE_SANDBOX_ENABLED && (nth_dir = check_prefix_dir(path,SANDBOX_PATH_EXTERNAL)) > 0) { */
	/* 	//external file storage sandbox */
	/* 	char new_path[len + 1]; */
	/* 	//replace dir in path with LINK_PREFIX */
	/* 	char* second_dir = get_nth_dir(path, nth_dir + 1); */
	/* 	strcpy(new_path, SANDBOX_LINK); */
	/* 	strcat(new_path, second_dir); */
	/* 	ptrace_write_data(pid, new_path, arg0, len + 1); */
	/* 	LOGD("pid %d %s: %s\n ==> new path: %s\n", pid, syscall, path, new_path); */

	/* 	// return from open syscall, reset the path */
	/* 	pid = waitpid(pid, NULL, __WALL); */

	/* 	ptrace_write_data(pid, path, arg0, len + 1); */

	/* 	return; */
	}
#endif
	LOGD("[%d] %s(%s, ...)\n", pid, syscall, path);
}

static void _handle_syscall_open(sandbox_t *sandbox, pid_t pid)
{
	tracee_ptr_t path_ptr = (tracee_ptr_t) ptrace_get_syscall_arg(pid, 0);
	int oflag = (int) ptrace_get_syscall_arg(pid, 1);

	int len = ptrace_strlen(pid, path_ptr);
	char path[len + 1];
	ptrace_read_data(pid, path, path_ptr, len + 1);

	if (strcmp(path, DEV_BINDER) == 0) {
		/* retrieve the fd from the syscall */
		// TODO
		// sandbox->binder_fd = (int) ptrace_get_syscall_arg(pid , 0);
		LOGD("[%d] open binder fd=%d\n", pid, sandbox->binder_fd);
	} else {
		_handle_file_series_syscalls(pid, "open", sandbox->flag, sandbox->uid, sandbox->gid);
	}
}

static void _handle_syscall_ioctl(sandbox_t *sandbox, pid_t pid)
{
	int fd = ptrace_get_syscall_arg(pid, 0);
	sandbox->binder_fd = find_fd(pid, DEV_BINDER);
	int binder_fd = sandbox->binder_fd;
	if (fd == sandbox->binder_fd) {
		LOGD("[%d] binder ioctl %d; binder_fd=%d\n", pid, fd, binder_fd);
		handle_binder(sandbox, pid);
	} else {
		LOGD("[%d] ioctl; binder_fd=%d\n", pid, binder_fd);
	}
}

//setuid/gid syscall handler
//return:
//	0 -- do not need to trace this process
//	1 -- need to trace
//and the arg will be set to the argument of this syscall
int syscall_setuid_gid_handler(pid_t pid, pid_t target, int is_gid, long* arg)
{
	// TODO
	//arg0 will be the uid/gid
	long arg0 = ptrace_get_syscall_arg(pid, 0);
	*arg = arg0;
	if(!PROCESS_FILTER_ENABLED || pid != target){
		return 1;
	}
	FILE *filter_file;
	if((filter_file = fopen(PROCESS_FILTER_PATH,"r")) == NULL){
		LOGD("Error: failed to open process filter file");
		return 1;
	}
	long uid,gid;
	int result = 0;
	while(!feof(filter_file)){
		fscanf(filter_file, "%ld %ld", &uid, &gid);
		if(is_gid && gid == arg0){
			result = 1;
			break;
		} else if(!is_gid && uid == arg0){
			result = 1;
			break;
		}
	}
	fclose(filter_file);

	return result;
}

void sandbox_handle_syscall(sandbox_t *sandbox, long scno, pid_t pid)
{
	long syscall_nr = scno;
	pid_t process_pid = pid;
	int flag = sandbox->flag;
	uid_t uid = sandbox->uid;
	gid_t gid = sandbox->gid;
	if (sandbox->hooks.before_handle_syscall) {
		sandbox->hooks.before_handle_syscall(sandbox, pid, syscall_nr);
	}
	switch (syscall_nr) {
		case __NR_stat:
			_handle_file_series_syscalls(pid, "stat", flag, uid, gid);
			break;
		case __NR_stat64:
			_handle_file_series_syscalls(pid, "stat64", flag, uid, gid);
			break;
		/* case __NR_newstat: */
		/* 	_handle_file_series_syscalls(pid, "newstat", flag, uid, gid); */
		/* 	break; */
		case __NR_lstat:
			_handle_file_series_syscalls(pid, "lstat", flag, uid, gid);
			break;
		case __NR_lstat64:
			_handle_file_series_syscalls(pid, "lstat64", flag, uid, gid);
			break;
		/* case __NR_newlstat: */
		/* 	_handle_file_series_syscalls(pid, "newlstat", flag, uid, gid); */
		/* 	break; */
		case __NR_readlink:
			_handle_file_series_syscalls(pid, "readlink", flag, uid, gid);
			break;
		case __NR_statfs:
			_handle_file_series_syscalls(pid, "statfs", flag, uid, gid);
			break;
		case __NR_statfs64:
			_handle_file_series_syscalls(pid, "statfs64", flag, uid, gid);
			break;
		case __NR_truncate:
			_handle_file_series_syscalls(pid, "truncate", flag, uid, gid);
			break;
		case __NR_truncate64:
			_handle_file_series_syscalls(pid, "truncate64", flag, uid, gid);
			break;
		case __NR_utimes:
			_handle_file_series_syscalls(pid, "utimes", flag, uid, gid);
			break;
		case __NR_access:
			_handle_file_series_syscalls(pid, "access", flag, uid, gid);
			break;
		case __NR_chdir:
			_handle_file_series_syscalls(pid, "chdir", flag, uid, gid);
			break;
		case __NR_chroot:
			_handle_file_series_syscalls(pid, "chroot", flag, uid, gid);
			break;
		case __NR_chmod:
			_handle_file_series_syscalls(pid, "chmod", flag, uid, gid);
			break;
		case __NR_chown:
			_handle_file_series_syscalls(pid, "chown", flag, uid, gid);
			break;
		case __NR_chown32:
			_handle_file_series_syscalls(pid, "chown32", flag, uid, gid);
			break;
		case __NR_lchown:
			_handle_file_series_syscalls(pid, "lchown", flag, uid, gid);
			break;
		case __NR_lchown32:
			_handle_file_series_syscalls(pid, "lchown32", flag, uid, gid);
			break;
		case __NR_open:
			_handle_syscall_open(sandbox, pid);
			break;
		case __NR_creat:
			_handle_file_series_syscalls(pid, "creat", flag, uid, gid);
			break;
		case __NR_mknod:
			_handle_file_series_syscalls(pid, "mknod", flag, uid, gid);
			break;
		case __NR_mkdir:
			_handle_file_series_syscalls(pid, "mkdir", flag, uid, gid);
			break;
		case __NR_rmdir:
			_handle_file_series_syscalls(pid, "rmdir", flag, uid, gid);
			break;
		case __NR_acct:
			_handle_file_series_syscalls(pid, "acct", flag, uid, gid);
			break;
		case __NR_uselib:
			_handle_file_series_syscalls(pid, "uselib", flag, uid, gid);
			break;
		case __NR_unlink:
			_handle_file_series_syscalls(pid, "unlink", flag, uid, gid);
			break;
		case __NR_ioctl:
			_handle_syscall_ioctl(sandbox, pid);
			break;
		case __NR_setuid32:
			/* TODO
			if (syscall_setuid_gid_handler(pid, process_pid, 0, (long *)&uid) == 0) {
				LOGD("app pid=%d exit\n", getpid());
			}
			*/
		case __NR_setgid32:
			/* TODO
			if (syscall_setuid_gid_handler(pid, process_pid, 1, (long *)&gid) == 0) {
				LOGD("app pid=%d exit\n", getpid());
			}
			*/
		case __NR_link:
			/*
			 * TODO: link syscall handler
			 */
			/*
		case __NR_utime:
			TODO */
		default:
			LOGD("[%d] other syscall: %ld\n", pid, syscall_nr);
			break;
	}
	if (sandbox->hooks.after_handle_syscall) {
		sandbox->hooks.after_handle_syscall(sandbox, pid, syscall_nr);
	}
}

sandbox_t *sandbox_init(sandbox_t *sandbox, int flag)
{
	sandbox->flag = flag;
	sandbox->binder_fd = -1;
	sandbox->uid = -1;
	sandbox->gid = -1;
	memset(&sandbox->hooks, 0, sizeof(sandbox->hooks));
	return sandbox;
}

