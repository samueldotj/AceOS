/*! \file	kernel/pm/scheduler.c
	\brief	Scheduler management
*/

#include <ace.h>
#include <ds/bits.h>
#include <sync/spinlock.h>
#include <kernel/processor.h>
#include <kernel/debug.h>
#include <kernel/pm/scheduler.h>
#include <kernel/pm/task.h>
#include <kernel/mm/kmem.h>

#define MAX_SCHED_BONUS 			( (INT8)(10) )
#define MIN_SCHED_BONUS 			( (INT8)(-10) )
#define THREAD_PRIORITY(thread_ptr)	( (thread_ptr)->priority_queue->priority ) 

static INT8 AddThreadToSchedulerQueue(THREAD_PTR in_thread);
static void SwapPriorityQueues(PRIORITY_QUEUE_PTR pqueue_low , PRIORITY_QUEUE_PTR pqueue_high);
static void IncrementSchedulerBonus(PRIORITY_QUEUE_PTR pqueue);
static void DecrementSchedulerBonus(PRIORITY_QUEUE_PTR pqueue);
static void PreemptThread(THREAD_PTR new_thread);
static void RemoveThreadFromSchedulerQueue(THREAD_PTR rem_thread);
static PROCESSOR_PTR SelectProcessorToRun(THREAD_PTR in_thread);

static void idle_thread_function();

/*!
 *	\brief	Swap the given priority queues. Only items to be manually swapped are priority and time_slice.
 *	\param	@pqueue_low		Pointer to a priority queue
 *			@pqueue_high	Pointer to a priority queue
 */
static void SwapPriorityQueues(PRIORITY_QUEUE_PTR pqueue_low , PRIORITY_QUEUE_PTR pqueue_high)
{
	UINT8 *plow, *phigh;

	assert( pqueue_low != NULL && pqueue_high != NULL );

	plow = &pqueue_low->priority;
	phigh = &pqueue_high->priority;

	pqueue_low->ready_queue->priority_queue[*plow] = pqueue_high;
	pqueue_high->ready_queue->priority_queue[*phigh] = pqueue_low;

	/*! Swap priority */
	SWAP( *plow, *phigh, UINT8 );
	
	/* Swap time_slice */
	SWAP( pqueue_low->time_slice, pqueue_high->time_slice, UINT8 );
}

/*!
 *	\brief	Increment the scheduler bonus field of the given priority queue because the given priority queue was preempted.
 *	\param	@pqueue		Pointer to a priority queue.
 */
static void IncrementSchedulerBonus(PRIORITY_QUEUE_PTR pqueue)
{
	PRIORITY_QUEUE_PTR adj_high_priority_queue;

	if( pqueue->bonus == MAX_SCHED_BONUS)
	{
		if( (pqueue->priority % SCHEDULER_PRIORITY_LEVELS_PER_CLASS) > SCHED_PRI_VERY_HIGH)
		{
			pqueue->bonus = 0;
			/* Swap the adjacent priority queues */
			adj_high_priority_queue = pqueue->ready_queue->priority_queue[pqueue->priority - 1];

			SwapPriorityQueues(pqueue, adj_high_priority_queue);
		}
	}
	else
		pqueue->bonus++ ;
}

/*!
 *	\brief	Decrement the scheduler bonus field of the given priority queue because a thread from the given priority queue just recently utilised it's entire timelice and occupied the cpu.
 *	\param	@pqueue		Pointer to a priority queue.
 */

static void DecrementSchedulerBonus(PRIORITY_QUEUE_PTR pqueue)
{
	PRIORITY_QUEUE_PTR adj_low_priority_queue;

	if( pqueue->bonus != MIN_SCHED_BONUS)
		pqueue->bonus-- ;
	else
		if( (pqueue->priority % SCHEDULER_PRIORITY_LEVELS_PER_CLASS) < SCHED_PRI_VERY_LOW)
		{
			pqueue->bonus = 0;
			/* Swap the adjacent priority queues */
			adj_low_priority_queue = pqueue->ready_queue->priority_queue[pqueue->priority + 1];

			SwapPriorityQueues(pqueue, adj_low_priority_queue);
		}
}

/*!
 *	\brief	This should replace the kernel stack and call ContextSwitch.
 *			Assumption: Before entering, hold lock on new_thread.
 *	\param	@new_thread:	Pointer to new thread which is to be run. 
*/
static void PreemptThread(THREAD_PTR new_thread)
{
	THREAD_PTR running_thread = GetCurrentThread();

	AddThreadToSchedulerQueue(running_thread);
	new_thread->current_processor = running_thread->current_processor;
	SwitchContext( STRUCT_ADDRESS_FROM_MEMBER( new_thread, THREAD_CONTAINER, thread ) );
}

/*!
 *	\brief	Add a thread to one of scheduler's priotiy queue, based on value of priority in the thread's priroty queue structure.
 *	\param	@in_thread		:   Pointer to thread which is to be added.
 *	\retval 0	:	success
 *			-1	:	error
*/
static INT8 AddThreadToSchedulerQueue(THREAD_PTR in_thread)
{
	PRIORITY_QUEUE_PTR pqueue;

	pqueue = in_thread->priority_queue; /* Get the priority queue into which the new thread is to be inserted. */

	SpinLock( &in_thread->lock );

	InitList(&in_thread->priority_queue_list); /* Make sure any dangling pointers are removed, by detaching this from any queue */	

	if(pqueue->thread_head == NULL)
	{
		pqueue->thread_head = in_thread; /* First thread in this queue */
		SetBitInBitArray(&pqueue->ready_queue->mask, pqueue->priority);
	}
	else
	{
		AddToListTail(&pqueue->thread_head->priority_queue_list, &in_thread->priority_queue_list); /* Add the given thread to the list of threads in this priority queue */
	}

	in_thread->time_slice = pqueue->time_slice;
	in_thread->state = THREAD_STATE_READY;

	SpinUnlock( &in_thread->lock );
	return 0;
}

/*!
 *	\brief	 Remove thread from the scheduler priority queue.
 *	\param	 @rem_thread: pointer to thread which is to be removed
*/
static void RemoveThreadFromSchedulerQueue(THREAD_PTR rem_thread)
{
	PRIORITY_QUEUE_PTR pqueue;

	pqueue = rem_thread->priority_queue; /* Get the priority queue into which the new thread is to be inserted. */
	
	SpinLock( &rem_thread->lock );
	
	rem_thread->state = THREAD_STATE_TRANSITION;
	
	if( IsListEmpty( &rem_thread->priority_queue_list ) ) /* if only 1 thread in the queue and that is to be removed now, header should be made NULL */
	{
		pqueue->thread_head = NULL;
		ClearBitInBitArray(&pqueue->ready_queue->mask, pqueue->priority);
	}
	else
	{
		/* If rem_thred was the first element in the queue, then we need to update the head */
		if( pqueue->thread_head == rem_thread ) 
			pqueue->thread_head = STRUCT_ADDRESS_FROM_MEMBER( rem_thread->priority_queue_list.next, THREAD, priority_queue_list);
			
		RemoveFromList( &rem_thread->priority_queue_list );
	}
	
	SpinUnlock( &rem_thread->lock );
}

/*!
 *	\brief	Modify the priority info of the thread and move it to the corresponding priority queue. 
 *	\param	@mod_thread		:	Pointer to thread whose priority is to be modified.
 *			@sched_class	:	New priority which is to be set to the thread.
 *	\retval return status of AddThreadToSchedulerQueue()
*/
INT8 ModifyThreadPriorityInfo(THREAD_PTR mod_thread, SCHEDULER_CLASS_LEVELS sched_class)
{
	if (sched_class == THREAD_PRIORITY(mod_thread) / 5) /* No need to move, already in the same class. */
		return 0;
	/* First remove the thread from it's current priority queue. */
	RemoveThreadFromSchedulerQueue(mod_thread);

	THREAD_PRIORITY(mod_thread) = (UINT8)(sched_class + SCHED_PRI_MID);

	if ( THREAD_PRIORITY(mod_thread) > THREAD_PRIORITY( GetCurrentThread() ) ) /*! New thread's priority is higher than the currently running thread's priority. Hence do a context switch */
	{
		PreemptThread(mod_thread);	/* This should replace the kernel stack and call ContextSwitch */
		return 0;
	}

	/* Now add the thread to the corresponding priority queue */
	return AddThreadToSchedulerQueue(mod_thread);
}

/*!
 *	\brief	Get the scheduler priority info of the thread. 
 *	\param	@my_thread:   pointer to thread whose priority is to be fetched. 
 *	\retval The scheduler class of the thread under inspection.
*/
SCHEDULER_CLASS_LEVELS GetThreadPriorityInfo(THREAD_PTR my_thread)
{
	return (SCHEDULER_CLASS_LEVELS)( (THREAD_PRIORITY(my_thread) ) / SCHEDULER_PRIORITY_LEVELS_PER_CLASS );
}

/*!
 *	\brief	Selects a new thread to run from the processor's ready queue's.
 *	\param	@hint	Current priority of the priority_queue on which
 *	\retval	THREAD_PTR 	Pointer to a thread which is selected to run on this CPU.
*/
static THREAD_PTR SelectThreadToRun(int hint)
{
	PROCESSOR_PTR		this_processor;
	UINT32				mask;
	UINT32 				result;
	THREAD_PTR			run_thread;
	PRIORITY_QUEUE_PTR	pqueue;

	this_processor = GET_CURRENT_PROCESSOR;

	mask = this_processor->active_ready_queue->mask;
	//mask = (this_processor->active_ready_queue->mask >> hint)<<hint;
	if( (result = FindFirstSetBitInBitArray(&mask, sizeof(mask)*BITS_PER_BYTE)) != -1)
	{
		pqueue = this_processor->active_ready_queue->priority_queue[result];
	}
	else 
	{
		/*! if not found, use dormant queue. */
		mask = this_processor->dormant_ready_queue->mask;
		//mask = (this_processor->dormant_ready_queue->mask >> hint)<<hint;
		if( (result=FindFirstSetBitInBitArray(&mask, sizeof(mask)*BITS_PER_BYTE)) != -1)
		{
			pqueue = this_processor->dormant_ready_queue->priority_queue[result];
		}
		else
		{
			/*! No threads available for running so select idle thread*/
			return GetCurrentThread()->current_processor->idle_thread;
		}
	}

	/*! get a thread from this queue to run */
	run_thread = pqueue->thread_head;
	
	assert(run_thread != NULL);	/*! if bit masks are properly updated, then run_thread should not be null at tis point of time. */
		
	RemoveThreadFromSchedulerQueue(run_thread);
	return run_thread;
}

/*!
 *	\brief	Selects a less loaded processor on which the given thread can be added to it's ready queue.
 *	\param	@in_thread:	Thread which is to be added to the ready queue.
 *	\retval	PROCESSOR_PTR	pointer to a processor structure.
 */
static PROCESSOR_PTR SelectProcessorToRun(THREAD_PTR in_thread)
{
	int loop;
	int	hint=0; /*! processor id of an online processor */
	/*! loop through all the processors and see if any processors are not heavily loaded. */
	for(loop=0 ; loop < MAX_PROCESSORS ; loop++)
	{
		if(processor[loop].state == PROCESSOR_STATE_ONLINE && processor[loop].loaded != 0)
			return &processor[loop];
		else if(processor[loop].state == PROCESSOR_STATE_ONLINE)
			hint = loop;
		else ;
	}

	if(in_thread->state != THREAD_STATE_NEW)
	{
		/*! This is not a new thread, so assign this to the processor on which it last ran. */
		if(in_thread->last_processor->state == PROCESSOR_STATE_ONLINE)
			return in_thread->last_processor;
	}

	/*! Select randomly one of the processors and add the thread on it's ready queue. */
	//todo
	return &processor[hint];
}

/*!
 *	\brief	This function is called when the running thread finishes it's quota or at interrupt context when a thread wants to move from BLOCKING state to READY state or when a new thread is created.
 *			If this is a new thread, scheduler class has to be assigned before calling this function.
 *	\param	@in_thread	Incoming thread which is either newly created or moving from blocking state. 
*/
void ScheduleThread(THREAD_PTR in_thread)
{
	THREAD_PTR new_thread, current_thread;
	PROCESSOR_PTR target_processor = NULL;
	UINT8 new_thread_priority;

	current_thread = GetCurrentThread();

	if(in_thread->state == THREAD_STATE_NEW)
	{
		/*  If the priority is higher than the current running thread, prempt it.
		 *  Or else just add it to dormant queue.
		 */
		new_thread_priority = (in_thread->priority * SCHEDULER_PRIORITY_LEVELS_PER_CLASS) + SCHED_PRI_MID;
		assert(new_thread_priority < MAX_SCHEDULER_PRIORITY_LEVELS); /*! make sure invalid thread level is not passed on */
		if ( in_thread->priority > THREAD_PRIORITY(current_thread) ) /*! New thread's priority is higher than the currently running thread's priority. Hence do a context switch */
		{
			/*! Since the current priority queue didn't get full quota to run, we should PROMOTE it. */
			IncrementSchedulerBonus(current_thread->priority_queue);
			PreemptThread(in_thread);	/* This should replace the kernel stack and call ContextSwitch */
			return;
		}
		/*! if the thread bound to a processor use it*/
		if ( in_thread->bind_cpu != (INT8) -1 )
			target_processor  = &processor[(int)in_thread->bind_cpu];
		if ( target_processor == NULL )
			target_processor = SelectProcessorToRun(in_thread);
			
		in_thread->priority_queue = target_processor->dormant_ready_queue->priority_queue[new_thread_priority];
		AddThreadToSchedulerQueue(in_thread);
	}
	else if(in_thread == current_thread) /*! Current thread's time quantum has expired. So select a suitable replacement */
	{
		new_thread = SelectThreadToRun(current_thread->priority_queue->priority);
		if(new_thread == NULL) /*! No other threads available for running. So continue running the current thread */
		{
			//kprintf("[Idle Thread] %p ", current_thread->current_processor );
			
			return;
		}
		
		/*! Since the current priority queue got full quota to run, we should DEPROMOTE it. */
		DecrementSchedulerBonus(current_thread->priority_queue); /*! This will try to decrement the bonus. But if we are the lowest, then decrease the priority of the thread. */

		PreemptThread(new_thread);
	}
	else
	{
		/*! The thread wants to move from BLOCKING state to READY state.
		 *  If the priority is higher than the current running thread, prempt it.
		 *  Or else just add it to dormant queue.
	 	 */
		if ( in_thread->priority > current_thread->priority ) /*! New thread's priority is higher than the currently running thread's priority. Hence do a context switch */
		{
			/*! Since the current priority queue didn't get full quota to run, we should PROMOTE it. */
			IncrementSchedulerBonus(current_thread->priority_queue);
			PreemptThread(in_thread);	/* This should replace the kernel stack and call ContextSwitch */
			return;
		}
		AddThreadToSchedulerQueue(in_thread);
	}
}

/*! Creates ready queue, initializes and returns it*/
static READY_QUEUE_PTR CreateReadyQueue()
{
	int loop;
	READY_QUEUE_PTR ready_queue = kmalloc(sizeof(READY_QUEUE), KMEM_NO_FAIL);
	for (loop=0; loop<MAX_SCHEDULER_PRIORITY_LEVELS ; loop++)
	{
		InitSpinLock( &ready_queue->lock );
		ready_queue->mask = 0;
		ready_queue->priority_queue[loop] = kmalloc(sizeof(PRIORITY_QUEUE), KMEM_NO_FAIL);
				
		InitSpinLock(&ready_queue->priority_queue[loop]->lock);
		ready_queue->priority_queue[loop]->ready_queue = ready_queue;
		ready_queue->priority_queue[loop]->thread_head = NULL;
		ready_queue->priority_queue[loop]->priority = loop;
	}
	return ready_queue;
}
/*! Initializes scheduler data structures.
*/
void InitScheduler()
{
	int i;

	/* initialize the ready queue of all processors*/
	for(i=0; i<MAX_PROCESSORS; i++)
	{
		processor[i].active_ready_queue = CreateReadyQueue();
		processor[i].dormant_ready_queue = CreateReadyQueue();
	}

	/* initialize master boot thread*/
	InitBootThread( master_processor_id );
	
	/*! create idle_thread for all the cpus regardless of their state because it might become online after sometime*/
	for(i=0; i<MAX_PROCESSORS; i++)
	{
		THREAD_CONTAINER_PTR tc;
		 
		/*create idle thread for this processor*/
		if ( (tc = CreateThread(idle_thread_function, SCHED_CLASS_LOW) ) == NULL )
			panic("Idle thread creation failed");
		processor[i].idle_thread = &tc->thread;
	}
}

/*! Idle thread for each processor*/
static void idle_thread_function()
{
	while(1)
	{
		
	}
}
