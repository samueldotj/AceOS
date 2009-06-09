/*!
    \file   file.c
    \brief  system call  for handling files
*/

#include <ace.h>
#include <string.h>
#include <kernel/debug.h>
#include <kernel/system_call_handler.h>

/*int mount (const char * fs, const char * path, unsigned flags);*/
UINT32 syscall_mount(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}

/*int umount (const char * path);*/
UINT32 syscall_umount(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}

/*int getmntinfo(struct statfs **mntbufp, int flags);*/
UINT32 syscall_getmntinfo(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}

