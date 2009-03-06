/*!
    \file   system_calls.c
    \brief  system call handler implementation
*/

#include <ace.h>
#include <kernel/system_call_handler.h>
#include <kernel/printf.h>
#include <kernel/error.h>
#include <string.h>

/* Declare the system calls here */
ERROR_CODE dummy_system_call(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval);


/* system_calls is an array of pointers, pointing to system call functions,
 * accepting SYSTEM_CALL_ARGS_PTR as it's argument and returning an integer.
 * Suffix the system call number to each new entry for easy reference in future.
 */
ERROR_CODE ( *(system_calls[]) )(SYSTEM_CALL_ARGS_PTR, UINT32*) = {
    dummy_system_call //0
};

const int max_system_calls = (sizeof(system_calls) / sizeof(int*));


ERROR_CODE dummy_system_call(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	kprintf("dummy system call called\n");
	kprintf("arg1=%d arg2=%d arg3=%d arg4=%d arg5=%d\n",
		sys_call_args->args[0],
		sys_call_args->args[1],
		sys_call_args->args[2],
		sys_call_args->args[3],
		sys_call_args->args[4] );

	*retval = 100;  /* This is for user program to interpret */
	return ERROR_SUCCESS;  /* Success . This is for sys call handler*/
}


int SystemCallCacheConstructor(void *buffer)
{
	memset(buffer, 0, sizeof(SYSTEM_CALL_ARGS));
	return 0;
}


int SystemCallCacheDestructor(void *buffer)
{
	return 0;
}
