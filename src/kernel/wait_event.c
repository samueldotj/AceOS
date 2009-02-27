/*!
\file 	kernel/wait_event.c
\brief	Provides wait event mechanism.
*/

#include <ace.h>
#include <kernel/wait_event.h>
#include <kernel/pm/scheduler.h>
#include <kernel/interrupt.h>
#include <kernel/mm/kmem.h>
#include <kernel/debug.h>
#include <assert.h>

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
		AddToList( &((*wait_queue)->in_queue), &(wait_event->in_queue) );
	}

	return wait_event;
}


/*!
\brief	Waits for a particular event to fire until specified timeout.
\param	event		Event on which we have to wait
\param	timeout		Units in milli secobds; If =0, no timeout, block until the event happens; >0, block until either event fires up or the timeout happens.
Returns 0 on success(event happened before timeout.) or -1 on failure(timeout happenned and no event showed up)
*/
int WaitForEvent(WAIT_EVENT_PTR event, int timeout)
{
	int ret_timeout;
	THREAD_PTR my_thread;

	assert( event->thread == GetCurrentThread() );	/* This is necessary to avoid rogue threads inducing sleep to innocent threads */

	if(timeout > 0) /* Wait until the event fires up or the timeout expires */
	{
		ret_timeout = Sleep(timeout); /* This will block for the specified time */

		if(ret_timeout > 0) /* We woke up because, some other event finished */
			return 0;
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
			return -1;
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

				SpinLock( &(thread->lock) );
				if(thread->state == THREAD_STATE_RUN)
				{
					thread->state = THREAD_STATE_EVENT_FIRED;	/* An intermediate state to instruct the thread not to sleep or block */
					SpinUnlock( &(thread->lock) );
				}
				else
				{
					SpinUnlock( &(thread->lock) );
					ResumeThread(thread);
				}
			}
			temp_wait_event->fired = 1;
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

		SpinLock( &(thread->lock) );
		if(thread->state == THREAD_STATE_RUN)
		{
			thread->state = THREAD_STATE_EVENT_FIRED;	/* An intermediate state to instruct the thread not to sleep or block */
			SpinUnlock( &(thread->lock) );
		}
		else
		{
			SpinUnlock( &(thread->lock) );
			ResumeThread(thread);
		}
	}
	temp_wait_event->fired = 1;
}


static void ClearRelatedWaitEvents(WAIT_EVENT_PTR event)
{
	WAIT_EVENT_PTR temp_wait_event;
	LIST_PTR temp_list1, temp_list2, head_list;

	head_list = &(event->thread_events);
	LIST_FOR_EACH_REMOVAL(temp_list1, temp_list2, head_list )
	{
		temp_wait_event = STRUCT_ADDRESS_FROM_MEMBER(temp_list1, WAIT_EVENT, thread_events);
		RemoveFromList( &(temp_wait_event->thread_events) );
		temp_wait_event->thread = NULL;
		head_list = temp_list2;
	}

	temp_wait_event = STRUCT_ADDRESS_FROM_MEMBER(head_list, WAIT_EVENT, thread_events);
	RemoveFromList( &(temp_wait_event->thread_events) );
	temp_wait_event->thread = NULL;
}
