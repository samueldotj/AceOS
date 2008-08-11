/*!
  \file		thread.c
  \brief	Thread management
*/

#include <pm/thread.h>

/*
Thread's execution context in kernel mode
	------- Page align
	thread structure
	------- Guard Page
	xxxxxxx
	------- Stack page
	kernel stack
*/

/*gets the current kernel stack position*/
#define GET_KERNEL_STACK(kstack) 		asm volatile("movl %%esp, %0":"=m"(kstack))

/*! Get Current Thread*/
THREAD_PTR GetCurrentThread()
{
	BYTE * kstack;
	GET_KERNEL_STACK( kstack );
	kstack = PAGE_ALIGN( kstack ) - (2 * PAGE_SIZE);
}
