/*!
    \file   scheduler.c
    \brief  system call related to scheduler control
*/

#include <ace.h>
#include <string.h>
#include <kernel/debug.h>
#include <kernel/system_call_handler.h>

UINT32 syscall_sched_setaffinity(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_sched_getaffinity(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_sched_setparam(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_sched_getparam(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_sched_setscheduler(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_sched_getscheduler(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_sched_yield(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_sched_get_priority_max(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_sched_get_priority_min(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_sched_rr_get_interval(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_rt_sigreturn(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_rt_sigaction(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_rt_sigprocmask(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_rt_sigpending(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_rt_sigtimedwait(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_rt_sigqueueinfo(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_rt_sigsuspend(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_sigreturn(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_sigprocmask(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
