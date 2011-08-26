/*!
    \file   terminal.c
    \brief  system call related to console control
*/

#include <ace.h>
#include <string.h>
#include <kernel/debug.h>
#include <kernel/pm/task.h>
#include <kernel/system_call_handler.h>

/*! find the pathname of a terminal
 * char *ttyname(int fildes);
 * tyname() function shall return a pointer to a string containing a null-terminated pathname of the terminal associated with file descriptor fildes.
 * */
UINT32 syscall_ttyname(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	TASK_PTR task;
	THREAD_PTR thread;
	thread = GetCurrentThread();
	task = thread->task;
	KTRACE("%d \n", sys_call_args->args[0]);
	
	strcpy( thread->user_scratch, "/device/console" );
		
	* retval = (UINT32)thread->user_scratch;
	return 0;
}
UINT32 syscall_isatty(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = 1;
	return 0;
}
