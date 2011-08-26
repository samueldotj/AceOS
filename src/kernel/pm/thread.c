/*!
  \file		kernel/pm/thread.c
  \brief	Thread management
*/

#include <ace.h>
#include <string.h>
#include <kernel/debug.h>
#include <kernel/mm/kmem.h>
#include <kernel/pm/task.h>
#include <kernel/pm/thread.h>

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
	\param task - parent of this thread
	\param start_address - starting functions address
	\param priority_class - thread's priority class 
	\param is_kernel_thread - if set creates kernel mode thread else user mode thread
	\param arch_arg - argument for architecture depended layer
*/
THREAD_CONTAINER_PTR CreateThread(TASK_PTR task, void * start_address, SCHEDULER_PRIORITY_LEVELS priority_class, BYTE is_kernel_thread, VADDR arch_arg)
{
	THREAD_CONTAINER_PTR thread_container;
	VADDR user_stack;
		
	assert( task != NULL );
	
	if ( (thread_container = AllocateBuffer( &thread_cache, CACHE_ALLOC_SLEEP) ) == NULL )
	{
		KTRACE("AllocateBuffer failed");
		return NULL;
	}
	
	SpinLock( &task->lock );
	thread_container->thread.task = task;
	/*if this is the first thread being created update the task's thread head*/
	if ( task->thread_head == NULL )
	{
		task->thread_head = &thread_container->thread;
	}
	else
	{
		AddToListTail( &task->thread_head->thread_queue, &thread_container->thread.thread_queue );
	}
	SpinUnlock( &task->lock );
	
	/*kernel mode threads wont have user mode stack*/
	if ( is_kernel_thread )
	{
		user_stack = NULL;
	}
	else
	{
		/*create user stack*/
		if ( AllocateVirtualMemory( task->virtual_map, &user_stack, 0, USER_STACK_SIZE, PROT_WRITE|PROT_READ, 0, NULL ) != ERROR_SUCCESS )
		{
			KTRACE("User stack allocation failed");
			return NULL;
		}
		
		/*allocate a page for scratch*/
		AllocateVirtualMemory( task->virtual_map, (VADDR *)&thread_container->thread.user_scratch, 0, PAGE_SIZE, PROT_READ|PROT_WRITE, 0, NULL );
		/*should we need assert here?*/
		assert( thread_container->thread.user_scratch != NULL );
	}
	
	/*architecture depended things*/
	FillThreadContext(thread_container, start_address, is_kernel_thread, user_stack, arch_arg);
	
	/*add to the scheduler*/
	thread_container->thread.priority = priority_class;
	ScheduleThread( &thread_container->thread );
	
	return thread_container;
}

/*! Terminates the current thread
	This function should be called by a thread to exit.
*/
void ExitThread()
{
	THREAD_PTR cur_thread = GetCurrentThread();
	assert(cur_thread->reference_count>0);
	
	SpinLock( &cur_thread->lock );
	cur_thread->state = THREAD_STATE_TERMINATE;
	SpinUnlock( &cur_thread->lock );
	
	ScheduleThread(cur_thread);
}

/*! Frees the datastructures used by the thread
	\param thread - thread to be freed
	This function will be called by scheduler when it detects a thread should be removed from queue and stops the thread
*/
void FreeThread(THREAD_PTR thread)
{
	assert(thread && thread->reference_count>=0);
	
	SpinLock( &thread->lock );
	thread->reference_count--;
	SpinUnlock( &thread->lock );
	if ( thread->reference_count == 0 )
	{
		FreeBuffer( STRUCT_ADDRESS_FROM_MEMBER( thread, THREAD_CONTAINER, thread ), &thread_cache );
	}
	else
	{
		WakeUpEvent( &thread->thread_event, WAIT_EVENT_WAKE_UP_ALL );
	}
}

/*! Suspends the current thread's execution
*/
void PauseThread()
{
	THREAD_PTR cur_thread = GetCurrentThread();
	
	SpinLock( &cur_thread->lock );
	if(cur_thread->state == THREAD_STATE_EVENT_FIRED)
	{
		cur_thread->state = THREAD_STATE_RUN;
		SpinUnlock( &cur_thread->lock );
	}
	else
	{
		cur_thread->state = THREAD_STATE_WAITING;
		SpinUnlock( &cur_thread->lock );
		
		/*Invoke scheduler in arch depended way*/
		InvokeScheduler();
	}
}
/*! Resumes a thread's execution
	\param thread - thread to be resumed
*/
void ResumeThread(THREAD_PTR thread)
{
	assert( thread != NULL );
	/*resume the thread only if it is blocked*/
	if (thread->state != THREAD_STATE_WAITING)
		return;
	SpinLock( &thread->lock );
	thread->state = THREAD_STATE_READY;
	SpinUnlock( &thread->lock );
	
	ScheduleThread(thread);
}

/*! Initializes the booting kernel thread on a processor*/
void InitBootThread(int boot_processor_id)
{
	THREAD_PTR boot_thread;
	THREAD_CONTAINER_PTR thread_container;
	PROCESSOR_PTR p;
	
	boot_thread = GetCurrentThread();
	p = &processor[boot_processor_id];
	thread_container = STRUCT_ADDRESS_FROM_MEMBER( boot_thread, THREAD_CONTAINER, thread );
	thread_container->kernel_stack_pointer = (BYTE *)((VADDR)(&thread_container->kernel_stack))+PAGE_SIZE;
	
	InitSpinLock( &boot_thread->lock );
	boot_thread->state = THREAD_STATE_RUN;
	
	boot_thread->reference_count = 1;
	boot_thread->current_processor = p;
	boot_thread->bind_cpu = boot_processor_id;
	
	boot_thread->priority = SCHED_CLASS_VERY_LOW;
	boot_thread->priority_queue =  p->dormant_ready_queue->priority_queue[boot_thread->priority];
	InitList( &boot_thread->priority_queue_list );
	
	/*For master for processor the following is done twice - once in InitKernelTask() and again here.
	It is needed for early boot vm support. And no harm in doing it :)*/
	boot_thread->task = &kernel_task;
	
}

/*! Waits for the thread event to fire
 *	\param thread 		Thread which should be monitored
 *	\param wait_time	How long monitoring can happen
 *  \note This function will increment the thread reference count, it is the responsibility of the caller to decrement it after done with the thread structure by calling FreeThread()
 */
ERROR_CODE WaitForThread(THREAD_PTR thread, int wait_time)
{
	ERROR_CODE ret = ERROR_SUCCESS;
	WAIT_EVENT_PTR	my_wait_event;
	UINT32 ret_time;
	
	/* Wait for this message queue */
	SpinLock( &thread->lock );
	thread->reference_count++;
	my_wait_event = AddToEventQueue( &thread->thread_event );
	SpinUnlock( &thread->lock );

	if(wait_time == MESSAGE_QUEUE_NO_WAIT)
		wait_time = 0;

	ret_time = WaitForEvent(my_wait_event, wait_time);

	if(ret_time == 0) /* no event fired and we timeout out */
		ret = ERROR_TIMEOUT;

	assert(my_wait_event->fired == 1);

	kfree(my_wait_event);
	
	return ret;
}


/*! Internal function used to initialize the thread structure*/
int ThreadCacheConstructor( void *buffer)
{
	THREAD_CONTAINER_PTR thread_container = (THREAD_CONTAINER_PTR) buffer;
	memset(buffer, 0, sizeof(THREAD_CONTAINER) );
	
	InitSpinLock( &thread_container->thread.lock );
	InitList( &thread_container->thread.thread_queue );
	thread_container->thread.current_processor = NULL;
	thread_container->thread.state = THREAD_STATE_NEW;
	thread_container->thread.reference_count = 1;
	
	return 0;
}
/*! Internal function used to clear the thread structure*/
int ThreadCacheDestructor( void *buffer)
{
	ThreadCacheConstructor(buffer);
	return 0;
}
