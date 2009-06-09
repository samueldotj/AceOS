/*!
    \file   statfs.c
    \brief  system call for handling statfs and fstatfs
*/

#include <ace.h>
#include <string.h>
#include <kernel/debug.h>
#include <kernel/system_call_handler.h>

/*int statfs (const char * path, struct statfs * buf)*/
UINT32 syscall_statfs(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}

/*int fstatfs (int fd, struct statfs *buf)*/
UINT32 syscall_fstatfs(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}

