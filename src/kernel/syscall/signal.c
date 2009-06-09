/*!    \file   signal.c    \brief  system call related to signal */#include <ace.h>#include <string.h>#include <kernel/debug.h>#include <kernel/system_call_handler.h>/*! signal managementvoid (*signal(int sig, void (*func)(int)))(int);*/UINT32 syscall_signal(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;}/*! examine and change signal actionint sigaction(int sig, const struct sigaction *act, struct sigaction *oact);*/UINT32 syscall_sigaction(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;}/*! wait for a signalint sigsuspend(const sigset_t *sigmask);*/UINT32 syscall_sigsuspend(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;}/*! examine pending signalsint sigpending(sigset_t *set); */UINT32 syscall_sigpending(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;}/*!\replace with sigprocmask and pthread_sigmask */UINT32 syscall_sgetmask(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;}/*!\replace with sigprocmask and pthread_sigmask */UINT32 syscall_ssetmask(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;}/*! set and get signal alternate stack contextint sigaltstack(const stack_t *restrict ss, stack_t *restrict oss); */UINT32 syscall_sigaltstack(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;}