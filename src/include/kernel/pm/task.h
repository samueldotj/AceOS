/*! \file include/kernel/pm/task.h
    \brief task related strcutrues and function declarations
*/

#ifndef _TASK_H_
#define _TASK_H_

#include <ace.h>
#include <ds/list.h>
#include <sync/spinlock.h>
#include <kernel/mm/vm.h>
#include <kernel/pm/thread.h>

#define KERNEL_STACK_SIZE	(PAGE_SIZE)

typedef struct thread THREAD, * THREAD_PTR;
typedef struct task
{
	SPIN_LOCK	 	lock;					/*lock for the entire structure*/
	int				reference_count;
	
	VIRTUAL_MAP_PTR	virtual_map;			/*virtual map for this task*/
	
	THREAD_PTR		thread_head;			/*threads in the same task*/
}TASK, * TASK_PTR;


typedef enum 
{
	THREAD_STATE_READY			= 1,		/*thread is ready to run*/
	THREAD_STATE_RUN			= 2,		/*thread is running*/
	THREAD_STATE_TERMINATE		= 3,		/*thread is terminating*/
	THREAD_STATE_SLEEP			= 4,		/*thread is sleeping*/
}THREAD_STATE;


struct thread
{
	SPIN_LOCK	 	lock;					/*lock for the entire structure*/
	int				reference_count;
	
	TASK_PTR		task;					/*back pointer to task*/
	LIST			thread_queue;			/*threads in the same task*/
	
	THREAD_STATE	thread_state;			/*Run State*/
};

/*
Thread's execution context in kernel mode
	------- Page Align
	Thread structure
	------- Page Align
	Guard Page
	------- Page Align
	Kernel Stack
*/
typedef struct kernel_stack
{
	THREAD		thread;
	BYTE 		guard_page[PAGE_SIZE] __attribute__ ((aligned ( PAGE_SIZE )));
	BYTE		kernel_stack[KERNEL_STACK_SIZE];
}KERNEL_STACK, * KERNEL_STACK_PTR;


#ifdef __cplusplus
    extern "C" {
#endif

THREAD_PTR GetCurrentThread();

#ifdef __cplusplus
	}
#endif

#endif
