/*!
  \file		kernel/pm/thread.c
  \brief	Thread management
*/

#include <ace.h>
#include <string.h>
#include <kernel/debug.h>
#include <kernel/mm/kmem.h>
#include <kernel/pm/task.h>

CACHE thread_cache;

/*! Get Current Thread
 * \return Current thread pointer
 */
THREAD_PTR GetCurrentThread()
{
	THREAD_CONTAINER_PTR thread_container;
	
	thread_container = STRUCT_ADDRESS_FROM_MEMBER( GetKernelStackPointer(), THREAD_CONTAINER, kernel_stack );
	return &thread_container->thread;
}

/*! Create a new thread
	\param start_address - starting functions address
	\param priority_class - thread's priority class 
*/
THREAD_CONTAINER_PTR CreateThread(void * start_address, SCHEDULER_PRIORITY_LEVELS priority_class)
{
	THREAD_CONTAINER_PTR thread_container;
	
	if ( (thread_container = AllocateBuffer( &thread_cache, 0 ) ) == NULL )
	{
		KTRACE("AllocateBuffer failed");
		return NULL;
	}

	FillThreadContext(thread_container, start_address);
	
	thread_container->thread.priority = priority_class;
	InitSpinLock( &thread_container->thread.lock );
	thread_container->thread.current_processor = NULL;
	thread_container->thread.state = THREAD_STATE_NEW;
	ScheduleThread( &thread_container->thread );
	
	return thread_container;
}

/*! Initializes the booting kernel thread on a processor*/
void InitBootThread(int boot_processor_id)
{
	THREAD_PTR boot_thread = GetCurrentThread(); 
	
	boot_thread->current_processor = &processor[boot_processor_id];
	boot_thread->priority = SCHED_CLASS_VERY_LOW;
	InitSpinLock( &boot_thread->lock );
	boot_thread->state = THREAD_STATE_NEW;
	boot_thread->current_processor = &processor[boot_processor_id];
	ScheduleThread( boot_thread );
}

/*! Internal function used to initialize the thread structure*/
int ThreadCacheConstructor( void *buffer)
{
	memset(buffer, 0, sizeof(THREAD_CONTAINER) );
	
	return 0;
}
/*! Internal function used to clear the thread structure*/
int ThreadCacheDestructor( void *buffer)
{
	return 0;
}

