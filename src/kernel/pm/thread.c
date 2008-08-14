/*!
  \file		thread.c
  \brief	Thread management
*/

#include <ace.h>
#include <kernel/pm/task.h>

/*gets the current kernel stack position*/
#define GET_KERNEL_STACK(kstack) 		asm volatile("movl %%esp, %0":"=m"(kstack))

/*! Get Current Thread
 * \return Current thread pointer
 */
THREAD_PTR GetCurrentThread()
{
	BYTE * kstack;
	KERNEL_STACK_PTR kernel_stack;
	
	GET_KERNEL_STACK( kstack );
	kernel_stack = STRUCT_ADDRESS_FROM_MEMBER( kstack, KERNEL_STACK, kernel_stack );
	return &kernel_stack->thread;
}
