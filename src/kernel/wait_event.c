/*!
\file 	kernel/wait_event.c
\brief	Provides wait event mechanism.
*/

#include <ace.h>
#include <assert.h>
#include <kernel/debug.h>
#include <kernel/interrupt.h>
#include <kernel/wait_event.h>
#include <kernel/mm/kmem.h>
#include <kernel/pm/scheduler.h>
#include <kernel/pm/timeout_queue.h>
#include <kernel/pm/thread.h>

static void ClearRelatedWaitEvents(WAIT_EVENT_PTR event);

/*!
\brief	Creates and adds an event to the wait queue.
\param	wait_queue	Wait queue inside which the thread has to wait.
*/
WAIT_EVENT_PTR AddToEventQueue(WAIT_EVENT_PTR *wait_queue)
{
	WAIT_EVENT_PTR wait_event = kmalloc(sizeof(WAIT_EVENT), KMEM_NO_FAIL);
	wait_event->thread = GetCurrentThread();
	InitList( &(wait_event->in_queue) );
	InitList( &(wait_event->thread_events) );
	wait_event->fired = 0;

	if(*wait_queue == NULL)
		*wait_queue = wait_event;
	else
	{
		AddToListTail( &((*wait_queue)->in_queue), &(wait_event->in_queue) );
	}

	return wait_event;
}


/*!
\brief	Waits for a particular event to fire until specified timeout.
\param	event		Event on which we have to wait
\param	timeout		Units in milli secobds; If =0, no timeout, block until the event happens; >0, block until either event fires up or the timeout happens.
Returns >0(remaining time) if event happened before timeout or 0 on failure(timeout happenned and no event showed up)
*/
UINT32 WaitForEvent(WAIT_EVENT_PTR event, UINT32 timeout)
{
	int ret_timeout;
	THREAD_PTR my_thread;

	assert( event->thread == GetCurrentThread() );	/* This is necessary to avoid rogue threads inducing sleep to innocent threads */
	
	if(timeout > 0) /* Wait until the event fires up or the timeout expires */
	{
		ret_timeout = Sleep(timeout); /* This will block for the specified time */
		if(ret_timeout > 0) /* We woke up because, some other event finished */
		{
			/* Removes the registered timeout event from timout queue */
			if (RemoveFromTimeoutQueue() == -1)
				panic("timeout queue corrupted\n");
			return ret_timeout;
		}
		else /* Timeout happenned and no event fired up. */
		{
			my_thread = (THREAD_PTR)(event->thread);
			if(my_thread != NULL) /* Nobody has yet cleared the events */
			{
				SpinLock( &(my_thread->wait_event_queue_lock) );
				ClearRelatedWaitEvents(event);
				event->thread = NULL;
				SpinUnlock( &(my_thread->wait_event_queue_lock) );
			}
			return 0;
		}
	}
	else /* Wait until the event fires up */
	{
		PauseThread();
		return 0;
	}
}

/*!
\brief	Wake up any other threads blocking on this event.
\param	event	Wake up threads waiting on this event.
\param	flag	If it contains WAIT_EVENT_WAKE_UP_ALL, wake up all threads waiting on this event, else wake up only 1st thread in queue.
Before entering here a lock should be taken for this wait event queue
*/
void WakeUpEvent(WAIT_EVENT_PTR *event, int flag)
{
	LIST_PTR temp_list1, temp_list2, head_list;
	WAIT_EVENT_PTR temp_wait_event;
	THREAD_PTR thread;

	assert( event!=NULL );
	/*if no one is waiting return*/
	if ( *event == NULL )
		return;
	head_list = &((*event)->in_queue) ;

	if(flag & WAIT_EVENT_WAKE_UP_ALL)
	{
		//kprintf("WakeUpEvent: Waking up all threads\n");
		LIST_FOR_EACH_REMOVAL(temp_list1, temp_list2, head_list)
		{
			if(temp_list1 == head_list)
				continue;
			temp_wait_event = STRUCT_ADDRESS_FROM_MEMBER(temp_list1, WAIT_EVENT, in_queue);
			RemoveFromList( &(temp_wait_event->in_queue) );
			thread = (THREAD_PTR)(temp_wait_event->thread);
			if(thread != NULL)
			{
				SpinLock( &(thread->wait_event_queue_lock) );
				ClearRelatedWaitEvents(temp_wait_event);
				temp_wait_event->thread = NULL;
				SpinUnlock( &(thread->wait_event_queue_lock) );
				temp_wait_event->fired = 1;

				SpinLock( &(thread->lock) );
				if(thread->state == THREAD_STATE_RUN)
				{
					thread->state = THREAD_STATE_EVENT_FIRED;	/* An intermediate state to instruct the thread not to sleep or block */
					//kprintf("special case!! event fired while thread about to sleep\n");
					SpinUnlock( &(thread->lock) );
				}
				else
				{
					SpinUnlock( &(thread->lock) );
					//kprintf("Event %p waking thread %p\n", temp_wait_event, thread);
					ResumeThread(thread);
				}
			}
		}
		temp_wait_event = *event;
		*event = NULL; /* This is the queue pointer from the structure. The wait_event is still active and should be freed after the thread is woken up*/
	}
	else
	{
		temp_wait_event = *event;
		if(!IsListEmpty(head_list))
			RemoveFromList(head_list);
		else
			*event = NULL;
		
	}

	/* Now wake up the first thread in wait event queue, because we had skipped that in the previous loop */
	thread = (THREAD_PTR)(temp_wait_event->thread);
	if(thread != NULL)
	{
		SpinLock( &(thread->wait_event_queue_lock) );
		ClearRelatedWaitEvents(temp_wait_event);
		temp_wait_event->thread = NULL;
		SpinUnlock( &(thread->wait_event_queue_lock) );
		temp_wait_event->fired = 1;

		SpinLock( &(thread->lock) );
		if(thread->state == THREAD_STATE_RUN)
		{
			thread->state = THREAD_STATE_EVENT_FIRED;	/* An intermediate state to instruct the thread not to sleep or block */
			//kprintf("special case!! event fired while thread: %p about to sleep\n", thread);
			SpinUnlock( &(thread->lock) );
		}
		else
		{
			SpinUnlock( &(thread->lock) );
			//kprintf("Event %p waking thread %p\n", temp_wait_event, thread);
			ResumeThread(thread);
		}
	}
}

/*!
 * \brief	Clears related threade events.
 * \param	event	Wait event for which the related thread events are to be cleared
 * Note: Only the THREAD pointer in event is made NULL and the event is disengaged from the thread events LIST.
 */
static void ClearRelatedWaitEvents(WAIT_EVENT_PTR event)
{
	WAIT_EVENT_PTR temp_wait_event;
	LIST_PTR temp_list1, temp_list2, head_list;

	head_list = &(event->thread_events);
	LIST_FOR_EACH_REMOVAL(temp_list1, temp_list2, head_list )
	{
		if(temp_list1 == head_list)
			continue;
		temp_wait_event = STRUCT_ADDRESS_FROM_MEMBER(temp_list1, WAIT_EVENT, thread_events);
		RemoveFromList( &(temp_wait_event->thread_events) );
		//kprintf("Event: %p Will not wake up thread: %p\n", temp_wait_event, temp_wait_event->thread);
		temp_wait_event->thread = NULL;
		//head_list = temp_list2;
	}

	//temp_wait_event = STRUCT_ADDRESS_FROM_MEMBER(head_list, WAIT_EVENT, thread_events);
	//RemoveFromList( &(temp_wait_event->thread_events) );
	//temp_wait_event->thread = NULL;
}


/*!
 * \brief Removes the event from the given queue. This call is done after an related event was fired or if an timeout happened.
 * \param	wait_event	Event that has to be removed.
 * \param	wait_queue	Queue from which the event is to be removed
 * NOTE: IN queue lock in the parent structure has to be taken before entering this function.
 */
void RemoveEventFromQueue(WAIT_EVENT_PTR search_wait_event, WAIT_EVENT_PTR *wait_queue)
{
	LIST_PTR temp1;
	WAIT_EVENT_PTR temp_event;
	
	assert( search_wait_event != NULL );
	assert( wait_queue != NULL );
	assert( *wait_queue != NULL );
	if(*wait_queue == search_wait_event)
	{
		if( IsListEmpty( &(search_wait_event->in_queue) ) )
			*wait_queue = NULL;
		else
			RemoveFromList( &((*wait_queue)->in_queue) );

		return;
	}

	LIST_FOR_EACH(temp1, &((*wait_queue)->in_queue) )
	{
		temp_event = STRUCT_ADDRESS_FROM_MEMBER(temp1, WAIT_EVENT, in_queue);
		if(temp_event == search_wait_event)
		{
			assert(temp_event->thread == NULL); /* make sure that this event is dormant */
			RemoveFromList( &temp_event->in_queue );
			return;
		}
	}
}
