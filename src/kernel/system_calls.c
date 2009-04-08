/*!
    \file   system_calls.c
    \brief  system call handler implementation
*/

#include <ace.h>
#include <string.h>
#include <kernel/debug.h>
#include <kernel/system_call_handler.h>
#include <kernel/printf.h>
#include <kernel/error.h>
#include <kernel/pm/thread.h>
#include <kernel/mm/vm.h>
#include <kernel/pm/task.h>


/* Declare the system calls here */
UINT32 dummy_system_call(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);
UINT32 syscall_exit(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);
UINT32 syscall_fstat(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);
UINT32 syscall_read(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);
UINT32 syscall_write(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);
UINT32 syscall_allocate_virtual_memory(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);

/* system_calls is an array of pointers, pointing to system call functions,
 * accepting SYSTEM_CALL_ARGS_PTR as it's argument and returning an integer.
 * Suffix the system call number to each new entry for easy reference in future.
 */
UINT32 ( *(system_calls[]) )(SYSTEM_CALL_ARGS_PTR, UINT32*) = {
	/*SYS_EXIT=0*/ syscall_exit,
	/*SYS_FORK*/ dummy_system_call,
	/*SYS_READ*/ syscall_read,
	/*SYS_WRITE*/ syscall_write,
	/*SYS_OPEN*/ dummy_system_call,
	/*SYS_CLOSE*/ dummy_system_call,
	/*SYS_WAITPID*/ dummy_system_call,
	/*SYS_CREAT*/ dummy_system_call,
	/*SYS_LINK*/ dummy_system_call,
	/*SYS_UNLINK*/ dummy_system_call,
	/*SYS_EXECVE*/ dummy_system_call,
	/*SYS_CHDIR*/ dummy_system_call,
	/*SYS_TIME*/ dummy_system_call,
	/*SYS_MKNOD*/ dummy_system_call,
	/*SYS_CHMOD*/ dummy_system_call,
	/*SYS_STAT*/ dummy_system_call,
	/*SYS_LSEEK*/ dummy_system_call,
	/*SYS_GETPID*/ dummy_system_call,
	/*SYS_MOUNT*/ dummy_system_call,
	/*SYS_STIME*/ dummy_system_call,
	/*SYS_PTRACE*/ dummy_system_call,
	/*SYS_ALARM*/ dummy_system_call,
	/*SYS_FSTAT*/ dummy_system_call,
	/*SYS_PAUSE*/ dummy_system_call,
	/*SYS_UTIME*/ dummy_system_call,
	/*SYS_ACCESS*/ dummy_system_call,
	/*SYS_NICE*/ dummy_system_call,
	/*SYS_SYNC*/ dummy_system_call,
	/*SYS_KILL*/ dummy_system_call,
	/*SYS_RENAME*/ dummy_system_call,
	/*SYS_MKDIR*/ dummy_system_call,
	/*SYS_RMDIR*/ dummy_system_call,
	/*SYS_DUP*/ dummy_system_call,
	/*SYS_PIPE*/ dummy_system_call,
	/*SYS_TIMES*/ dummy_system_call,
	/*SYS_BRK*/ dummy_system_call,
	/*SYS_SIGNAL*/ dummy_system_call,
	/*SYS_IOCTL*/ dummy_system_call,
	/*SYS_FCNTL*/ dummy_system_call,
	/*SYS_SETPGID*/ dummy_system_call,
	/*SYS_UMASK*/ dummy_system_call,
	/*SYS_CHROOT*/ dummy_system_call,
	/*SYS_USTAT*/ dummy_system_call,
	/*SYS_DUP2*/ dummy_system_call,
	/*SYS_GETPPID*/ dummy_system_call,
	/*SYS_GETPGRP*/ dummy_system_call,
	/*SYS_SETSID*/ dummy_system_call,
	/*SYS_SIGACTION*/ dummy_system_call,
	/*SYS_SGETMASK*/ dummy_system_call,
	/*SYS_SSETMASK*/ dummy_system_call,
	/*SYS_SIGSUSPEND*/ dummy_system_call,
	/*SYS_SIGPENDING*/ dummy_system_call,
	/*SYS_SETHOSTNAME*/ dummy_system_call,
	/*SYS_SETRLIMIT*/ dummy_system_call,
	/*SYS_GETRUSAGE*/ dummy_system_call,
	/*SYS_GETTIMEOFDAY*/ dummy_system_call,
	/*SYS_SETTIMEOFDAY*/ dummy_system_call,
	/*SYS_SYMLINK*/ dummy_system_call,
	/*SYS_LSTAT*/ dummy_system_call,
	/*SYS_READLINK*/ dummy_system_call,
	/*SYS_USELIB*/ dummy_system_call,
	/*SYS_SWAPON*/ dummy_system_call,
	/*SYS_REBOOT*/ dummy_system_call,
	/*SYS_MUNMAP*/ dummy_system_call,
	/*SYS_TRUNCATE*/ dummy_system_call,
	/*SYS_FTRUNCATE*/ dummy_system_call,
	/*SYS_FCHMOD*/ dummy_system_call,
	/*SYS_SOCKETCALL*/ dummy_system_call,
	/*SYS_SYSLOG*/ dummy_system_call,
	/*SYS_SETITIMER*/ dummy_system_call,
	/*SYS_GETITIMER*/ dummy_system_call,
	/*SYS_UNAME*/ dummy_system_call,
	/*SYS_WAIT4*/ dummy_system_call,
	/*SYS_SWAPOFF*/ dummy_system_call,
	/*SYS_SYSINFO*/ dummy_system_call,
	/*SYS_IPC*/ dummy_system_call,
	/*SYS_FSYNC*/ dummy_system_call,
	/*SYS_SIGRETURN*/ dummy_system_call,
	/*SYS_CLONE*/ dummy_system_call,
	/*SYS_MPROTECT*/ dummy_system_call,
	/*SYS_SIGPROCMASK*/ dummy_system_call,
	/*SYS_FCHDIR*/ dummy_system_call,
	/*SYS_SYSFS*/ dummy_system_call,
	/*SYS_SELECT*/ dummy_system_call,
	/*SYS_FLOCK*/ dummy_system_call,
	/*SYS_MSYNC*/ dummy_system_call,
	/*SYS_READV*/ dummy_system_call,
	/*SYS_WRITEV*/ dummy_system_call,
	/*SYS_SYSCTL*/ dummy_system_call,
	/*SYS_MLOCK*/ dummy_system_call,
	/*SYS_MUNLOCK*/ dummy_system_call,
	/*SYS_MLOCKALL*/ dummy_system_call,
	/*SYS_MUNLOCKALL*/ dummy_system_call,
	/*SYS_SCHED_SETPARAM*/ dummy_system_call,
	/*SYS_SCHED_GETPARAM*/ dummy_system_call,
	/*SYS_SCHED_SETSCHEDULER*/ dummy_system_call,
	/*SYS_SCHED_GETSCHEDULER*/ dummy_system_call,
	/*SYS_SCHED_YIELD*/ dummy_system_call,
	/*SYS_SCHED_GET_PRIORITY_MAX*/ dummy_system_call,
	/*SYS_SCHED_GET_PRIORITY_MIN*/ dummy_system_call,
	/*SYS_SCHED_RR_GET_INTERVAL*/ dummy_system_call,
	/*SYS_NANOSLEEP*/ dummy_system_call,
	/*SYS_MREMAP*/ dummy_system_call,
	/*SYS_POLL*/ dummy_system_call,
	/*SYS_NFSSERVCTL*/ dummy_system_call,
	/*SYS_PRCTL*/ dummy_system_call,
	/*SYS_RT_SIGRETURN*/ dummy_system_call,
	/*SYS_RT_SIGACTION*/ dummy_system_call,
	/*SYS_RT_SIGPROCMASK*/ dummy_system_call,
	/*SYS_RT_SIGPENDING*/ dummy_system_call,
	/*SYS_RT_SIGTIMEDWAIT*/ dummy_system_call,
	/*SYS_RT_SIGQUEUEINFO*/ dummy_system_call,
	/*SYS_RT_SIGSUSPEND*/ dummy_system_call,
	/*SYS_PREAD64*/ dummy_system_call,
	/*SYS_PWRITE64*/ dummy_system_call,
	/*SYS_CHOWN16*/ dummy_system_call,
	/*SYS_GETCWD*/ dummy_system_call,
	/*SYS_CAPGET*/ dummy_system_call,
	/*SYS_CAPSET*/ dummy_system_call,
	/*SYS_SIGALTSTACK*/ dummy_system_call,
	/*SYS_SENDFILE*/ dummy_system_call,
	/*SYS_VFORK*/ dummy_system_call,
	/*SYS_GETRLIMIT*/ dummy_system_call,
	/*SYS_MMAP2*/ dummy_system_call,
	/*SYS_TRUNCATE64*/ dummy_system_call,
	/*SYS_FTRUNCATE64*/ dummy_system_call,
	/*SYS_LCHOWN*/ dummy_system_call,
	/*SYS_MADVISE*/ dummy_system_call,
	/*SYS_FCNTL64*/ dummy_system_call,
	/*SYS_SCHED_SETAFFINITY*/ dummy_system_call,
	/*SYS_SCHED_GETAFFINITY*/ dummy_system_call,
	/*SYS_TIMER_CREATE*/ dummy_system_call,
	/*SYS_TIMER_SETTIME*/ dummy_system_call,
	/*SYS_TIMER_GETTIME*/ dummy_system_call,
	/*SYS_TIMER_GETOVERRUN*/ dummy_system_call,
	/*SYS_TIMER_DELETE*/ dummy_system_call,
	/*SYS_CLOCK_SETTIME*/ dummy_system_call,
	/*SYS_CLOCK_GETTIME*/ dummy_system_call,
	/*SYS_CLOCK_GETRES*/ dummy_system_call,
	/*SYS_CLOCK_NANOSLEEP*/ dummy_system_call,
	/*SYS_STATFS64*/ dummy_system_call,
	/*SYS_FSTATFS64*/ dummy_system_call,
	/*SYS_TGKILL*/ dummy_system_call,
	/*SYS_UTIMES*/ dummy_system_call,
	/*SYS_FADVISE64_64*/ dummy_system_call,
	/*SYS_MBIND*/ dummy_system_call,
	/*SYS_GET_MEMPOLICY*/ dummy_system_call,
	/*SYS_SET_MEMPOLICY*/dummy_system_call,
	/*SYS_ALLOCATE_VIRTUAL_MEMORY*/ syscall_allocate_virtual_memory
};

const int max_system_calls = (sizeof(system_calls) / sizeof(system_calls[0]));

UINT32 dummy_system_call(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("args[0]=%p args[1]=%p args[2]=%p arg4=%p arg5=%p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3],	sys_call_args->args[4] );

	*retval = 0;  /* This is for user program to interpret */
	return 0;  /* Success . This is for sys call handler*/
}
UINT32 syscall_exit(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	ExitThread();
	/*Not reached*/
	return -1;
}
UINT32 syscall_fstat(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("(file=%p struct stat=%p)\n", sys_call_args->args[0], sys_call_args->args[1]);

	*retval = -1;
	return -1;
}
UINT32 syscall_read(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("(file=%d buf=%p size=%d)\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[1]);
	*retval = -1;
	return -1;
}
UINT32 syscall_write(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("(file=%d buf=%p size=%d)\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2]);
	int i=0;
	
	char * buf = (char *)sys_call_args->args[1];
	while(i<sys_call_args->args[2] && buf)
		kprintf("%c", buf[i++]);
	*retval = -1;
	return -1;
}
/*! System call to allocate virtual memory to current user process
	\input args[0] - size
	\input args[1] - protection
	\input args[2] - preferred va start
*/
UINT32 syscall_allocate_virtual_memory(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	VADDR va;
	ERROR_CODE err;
	
	UINT32 size, protection, preferred_va;
	size = PAGE_ALIGN_UP(sys_call_args->args[0]);
	protection = PAGE_ALIGN_UP(sys_call_args->args[1]);
	preferred_va = PAGE_ALIGN_UP(sys_call_args->args[2]);
	
	KTRACE("(size=%d protection=%d preferred_va=%p)\n", size, protection, preferred_va);
	err = AllocateVirtualMemory( GetCurrentVirtualMap(), &va, preferred_va, size, protection, NULL, NULL ) ;
	if ( err == ERROR_SUCCESS )
		*retval = va;
	else
		*retval = NULL;
	return err;
}


