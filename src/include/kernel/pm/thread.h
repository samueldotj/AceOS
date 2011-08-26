/*! \file include/kernel/pm/thread.h
    \brief thread and kernel stack related strcutrues and function declarations
*/

#ifndef _THREAD_H_
#define _THREAD_H_

#include <ace.h>
#include <ds/list.h>
#include <sync/spinlock.h>
#include <kernel/processor.h>
#include <kernel/mm/vm.h>
#include <kernel/mm/kmem.h>
#include <kernel/pm/pm_types.h>
#include <kernel/pm/scheduler.h>
#include <kernel/pm/timeout_queue.h>

/*\todo remove these macros and put it as tunable*/
#define THREAD_CACHE_FREE_SLABS_THRESHOLD	100 
#define THREAD_CACHE_MIN_SLABS				10
#define THREAD_CACHE_MAX_SLABS				50

#define KERNEL_STACK_SIZE	(PAGE_SIZE)
#define USER_STACK_SIZE		(2*PAGE_SIZE)

typedef enum 
{
	THREAD_STATE_READY			= 1,		/*! thread is ready to run */
	THREAD_STATE_RUN			= 2,		/*! thread is running */
	THREAD_STATE_TERMINATE		= 3,		/*! thread is terminating */
	THREAD_STATE_WAITING		= 4,		/*! thread is waiting */
	THREAD_STATE_TRANSITION		= 5,		/*! thread is in transition phase from one state to another */
	THREAD_STATE_EVENT_FIRED	= 6,
	THREAD_STATE_NEW			= 7
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
	
	WAIT_EVENT_PTR			thread_event;			/*! Wait for this thread to finish*/

	/* Timeout queue */
	TIMEOUT_QUEUE			timeout_queue;

	/* Wait event queue lock */
	SPIN_LOCK				wait_event_queue_lock;	/*! Anybody accessing any wait events belonging to this structure or count_wait_event_queue variable should take this lock */
	
	/*ipc reply*/
	THREAD_PTR				ipc_reply_to_thread;	/*! Last message came from which thread(ie to which thread i have to reply)*/
	WAIT_EVENT_PTR			ipc_reply_event;		/*! Waitevent to wait to receive message(reply)*/
	MESSAGE_BUFFER			ipc_reply_message;		/*! buffer to receive reply data*/
	
	void *					arch_data;				/*! architecture depended data*/
	
	char * 					user_scratch;				/*! temporary user-mode mapped address to copy kernel content to user*/
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
struct thread_container
{
	THREAD		thread;							__attribute__ ((aligned ( PAGE_SIZE )))
	BYTE *		kernel_stack_pointer;
	BYTE 		guard_page[PAGE_SIZE] 			__attribute__ ((aligned ( PAGE_SIZE )));
	BYTE		kernel_stack[KERNEL_STACK_SIZE] __attribute__ ((aligned ( PAGE_SIZE )));
};

extern CACHE thread_cache;

#ifdef __cplusplus
    extern "C" {
#endif

int ThreadCacheConstructor(void *buffer);
int ThreadCacheDestructor(void *buffer);

inline BYTE * GetKernelStackPointer();
THREAD_PTR GetCurrentThread();

THREAD_CONTAINER_PTR CreateThread(TASK_PTR task, void * start_address, SCHEDULER_PRIORITY_LEVELS priority_class, BYTE is_kernel_thread, VADDR arch_arg);
void ExitThread();
void FreeThread(THREAD_PTR thread );
void PauseThread();
void ResumeThread(THREAD_PTR thread);

void InitBootThread(int boot_processor_id);

void FillThreadContext(THREAD_CONTAINER_PTR thread_container, void * start_address, BYTE is_kernel_thread, VADDR user_stack, VADDR arch_arg);
void SwitchContext(THREAD_CONTAINER_PTR thread_container);

ERROR_CODE WaitForThread(THREAD_PTR thread, int wait_time);

#ifdef __cplusplus
	}
#endif

#endif
