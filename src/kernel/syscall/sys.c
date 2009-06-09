/*!    \file   sys.c    \brief  system call related to system configuration and management*/#include <ace.h>#include <string.h>#include <kernel/debug.h>#include <kernel/system_call_handler.h>UINT32 syscall_sys_conf(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;}UINT32 syscall_getrlimit(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;}UINT32 syscall_umask(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;}UINT32 syscall_sethostname(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;}UINT32 syscall_setrlimit(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;}UINT32 syscall_symlink(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;}UINT32 syscall_swapon(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;}UINT32 syscall_swapoff(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;}UINT32 syscall_reboot(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;}UINT32 syscall_socketcall(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;}