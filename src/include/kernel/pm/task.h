/*! \file include/kernel/pm/task.h
    \brief task related strcutrues and function declarations
*/

#ifndef _TASK_H_
#define _TASK_H_

#include <ace.h>
#include <ds/list.h>
#include <sync/spinlock.h>
#include <kernel/mm/vm.h>
#include <kernel/mm/kmem.h>

typedef struct thread THREAD, * THREAD_PTR;

#include <kernel/processor.h>
#include <kernel/pm/scheduler.h>

#define KERNEL_STACK_SIZE	(PAGE_SIZE)

typedef struct task
{
	SPIN_LOCK	 	lock;					/*! lock for the entire structure */
	int				reference_count;
	
	VIRTUAL_MAP_PTR	virtual_map;			/*! virtual map for this task */
	
	THREAD_PTR		thread_head;			/*! threads in the same task */
}TASK, * TASK_PTR;

typedef enum 
{
	THREAD_STATE_READY			= 1,		/*! thread is ready to run */
	THREAD_STATE_RUN			= 2,		/*! thread is running */
	THREAD_STATE_TERMINATE		= 3,		/*! thread is terminating */
	THREAD_STATE_WAITING		= 4,		/*! thread is waiting */
	THREAD_STATE_TRANSITION		= 5,		/*! thread is in transition phase from one state to another */
	THREAD_STATE_NEW			= 6
}THREAD_STATE;


struct thread
{
	SPIN_LOCK	 			lock;					/*! lock for the entire structure */
	int						reference_count;
	
	TASK_PTR				task;					/*! back pointer to task */
	LIST					thread_queue;			/*! threads in the same task */
	
	PROCESSOR_PTR			current_processor;		/*! Pointer to current processor on which this thread is running */
	PROCESSOR_PTR			last_processor;			/*! Pointer to the processor on which this thread last ran */
	INT8					bind_cpu;				/*! CPU to which the thread is bound to run. -1 value implies thread is not bound */
	
	/* scheduler related data */
	THREAD_STATE			state;					/*! Run State */
	LIST					priority_queue_list;	/*! List of threads which are in the same priority queue */
	UINT8					time_slice;				/*! Time quantum for which the thread can be run */
	PRIORITY_QUEUE_PTR		priority_queue;			/*! Pointer to priority queue in either of active or dormant ready queue */
	SCHEDULER_CLASS_LEVELS	priority;				/*! External priority assigned by the user. This is used to select one of the scheduler classes */
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
typedef struct thread_container
{
	THREAD		thread;							__attribute__ ((aligned ( PAGE_SIZE )))
	BYTE *		kernel_stack_pointer;
	BYTE 		guard_page[PAGE_SIZE] 			__attribute__ ((aligned ( PAGE_SIZE )));
	BYTE		kernel_stack[KERNEL_STACK_SIZE] __attribute__ ((aligned ( PAGE_SIZE )));
}THREAD_CONTAINER, * THREAD_CONTAINER_PTR;

extern CACHE thread_cache;

#define THREAD_CACHE_FREE_SLABS_THRESHOLD	100 
#define THREAD_CACHE_MIN_SLABS				10
#define THREAD_CACHE_MAX_SLABS				50

#ifdef __cplusplus
    extern "C" {
#endif
int ThreadCacheConstructor(void *buffer);
int ThreadCacheDestructor(void *buffer);

inline BYTE * GetKernelStackPointer();
THREAD_PTR GetCurrentThread();

THREAD_CONTAINER_PTR CreateThread(void * start_address, SCHEDULER_PRIORITY_LEVELS priority_class);
void ExitThread();
void FreeThread(THREAD_PTR thread );

void InitBootThread(int boot_processor_id);

void FillThreadContext(THREAD_CONTAINER_PTR thread_container, void * start_address);
void SwitchContext(THREAD_CONTAINER_PTR thread_container);

#ifdef __cplusplus
	}
#endif

#endif
