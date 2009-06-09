/*!    \file   entry.c    \brief  system call handler array*/#include <ace.h>#include <string.h>#include <kernel/debug.h>#include <kernel/system_call_handler.h>/*syscall entry declations*/UINT32 syscall_exit(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_fork(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_vfork(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_waitpid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_execve(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_pause(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_nice(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_kill(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_brk(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_wait4(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_sys_conf(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_getrlimit(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_getcmdline(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_getenvironment(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_signal(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_sigaction(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_sigsuspend(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_sigpending(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_sgetmask(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_ssetmask(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_sigaltstack(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_read(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_write(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_open(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_close(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_creat(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_link(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_unlink(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_chdir(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_getcwd(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_mknod(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_chmod(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_fchmod(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_stat(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_lstat(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_lseek(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_mount(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_fstat(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_statfs64(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_fstatfs64(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_sync(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_rename(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_mkdir(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_rmdir(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_fchdir(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_sysfs(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_select(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_poll(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_dup(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_dup2(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_pipe(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_ioctl(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_fcntl(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_open_dir(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_close_dir(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_read_dir(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_read_dir_r(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_rewind_dir(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_seek_dir(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_tell_dir(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_truncate(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_ftruncate(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_pread64(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_pwrite64(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_sendfile(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_chroot(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_mount(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_umount(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_getmntinfo(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_statfs(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_fstatfs(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_umask(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_madvise(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_munmap(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_mremap(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_mmap2(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_mprotect(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_flock(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_msync(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_sysctl(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_mlock(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_munlock(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_mlockall(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_munlockall(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_time(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_stime(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_utimes(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_ptrace(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_alarm(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_utime(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_access(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_times(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_getrusage(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_gettimeofday(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_settimeofday(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_clock_settime(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_clock_gettime(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_clock_getres(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_clock_nanosleep(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_timer_create(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_timer_settime(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_timer_gettime(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_timer_getoverrun(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_timer_delete(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_setitimer(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_getitimer(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_nanosleep(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_sleep(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_sethostname(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_setrlimit(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_symlink(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_sched_setaffinity(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_sched_getaffinity(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_sched_setparam(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_sched_getparam(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_sched_setscheduler(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_sched_getscheduler(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_sched_yield(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_sched_get_priority_max(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_sched_get_priority_min(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_sched_rr_get_interval(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_rt_sigreturn(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_rt_sigaction(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_rt_sigprocmask(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_rt_sigpending(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_rt_sigtimedwait(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_rt_sigqueueinfo(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_rt_sigsuspend(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_sigreturn(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_sigprocmask(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_swapon(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_swapoff(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_reboot(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_socketcall(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_get_mempolicy(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_set_mempolicy(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_allocate_virtual_memory(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_uname(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_gethostid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_getlogin(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_getlogin_r(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_getuid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_geteuid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_getgid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_getegid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_getpgid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_getpgrp(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_getpid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_getppid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_getsid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_getpwuid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_setuid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_setgid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_setpgid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_setpgrp(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_setregid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_setreuid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_setsid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_ttyname(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);UINT32 syscall_isatty(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);/* !syscall table * system_calls is an array of pointers, pointing to system call functions, * accepting SYSTEM_CALL_ARGS_PTR as it's argument and returning an integer.  */UINT32 ( *(system_calls[]) )(SYSTEM_CALL_ARGS_PTR, UINT32*) = {	syscall_exit,	syscall_fork,	syscall_vfork,	syscall_waitpid,	syscall_execve,	syscall_pause,	syscall_nice,	syscall_kill,	syscall_wait4,	syscall_getcmdline,	syscall_getenvironment,			syscall_signal,	syscall_sigaction,	syscall_sigsuspend,	syscall_sigpending,	syscall_sgetmask,	syscall_ssetmask,	syscall_sigaltstack,	syscall_open,	syscall_close,	syscall_read,	syscall_write,	syscall_link,	syscall_unlink,	syscall_chdir,	syscall_getcwd,	syscall_mknod,	syscall_chmod,	syscall_fchmod,	syscall_stat,	syscall_lstat,	syscall_lseek,	syscall_fstat,	syscall_sync,	syscall_rename,	syscall_mkdir,	syscall_rmdir,	syscall_fchdir,	syscall_select,	syscall_poll,	syscall_dup,	syscall_dup2,	syscall_pipe,	syscall_ioctl,	syscall_fcntl,	syscall_open_dir,	syscall_close_dir,	syscall_read_dir,	syscall_read_dir_r,	syscall_rewind_dir,	syscall_seek_dir,	syscall_tell_dir,	syscall_truncate,	syscall_ftruncate,		syscall_mount,	syscall_umount,	syscall_getmntinfo,	syscall_statfs,	syscall_fstatfs,		syscall_umask,			syscall_madvise,	syscall_munmap,	syscall_mremap,	syscall_mmap2,	syscall_mprotect,	syscall_flock,	syscall_msync,	syscall_sysctl,	syscall_mlock,	syscall_munlock,	syscall_mlockall,	syscall_munlockall,			syscall_brk,	syscall_get_mempolicy,	syscall_set_mempolicy,	syscall_allocate_virtual_memory,			syscall_time,	syscall_stime,	syscall_utimes,	syscall_ptrace,	syscall_alarm,		syscall_utime,	syscall_access,		syscall_times,	syscall_getrusage,	syscall_gettimeofday,	syscall_settimeofday,	syscall_clock_settime,	syscall_clock_gettime,	syscall_clock_getres,	syscall_clock_nanosleep,	syscall_timer_create,	syscall_timer_settime,	syscall_timer_gettime,	syscall_timer_getoverrun,	syscall_timer_delete,	syscall_setitimer,	syscall_getitimer,	syscall_nanosleep,	syscall_sleep,		syscall_sethostname,	syscall_sys_conf,	syscall_getrlimit,	syscall_setrlimit,	syscall_symlink,		syscall_sched_setaffinity,	syscall_sched_getaffinity,	syscall_sched_setparam,	syscall_sched_getparam,	syscall_sched_setscheduler,	syscall_sched_getscheduler,	syscall_sched_yield,	syscall_sched_get_priority_max,	syscall_sched_get_priority_min,	syscall_sched_rr_get_interval,	syscall_rt_sigreturn,	syscall_rt_sigaction,	syscall_rt_sigprocmask,	syscall_rt_sigpending,	syscall_rt_sigtimedwait,	syscall_rt_sigqueueinfo,	syscall_rt_sigsuspend,	syscall_sigreturn,	syscall_sigprocmask,			syscall_swapon,	syscall_swapoff,		syscall_reboot,	syscall_socketcall,		syscall_uname,	syscall_gethostid,	syscall_getlogin,	syscall_getlogin_r,	syscall_getuid,	syscall_geteuid,	syscall_getgid,	syscall_getegid,	syscall_getpgid,	syscall_getpgrp,	syscall_getpid,	syscall_getppid,	syscall_getsid,	syscall_getpwuid,	syscall_setuid,	syscall_setgid,	syscall_setpgid,	syscall_setpgrp,	syscall_setregid,	syscall_setreuid,	syscall_setsid,		syscall_ttyname,	syscall_isatty,};const int max_system_calls = (sizeof(system_calls) / sizeof(system_calls[0]));