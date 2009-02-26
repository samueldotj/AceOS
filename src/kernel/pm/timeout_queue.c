/*! \file	kernel/pm/timeout_queue.c
	\brief	Manages Timeout queue routines which are necessary for sleeping and waking up threads at specified time. 
*/

#include <ace.h>
#include <ds/list.h>
#include <kernel/pm/timeout_queue.h>
#include <kernel/pm/task.h>
#include <kernel/pit.h>

volatile TIMEOUT_QUEUE_PTR	timeout_queue;

void ValuateTimeoutQueue(void)
{
	TIMEOUT_QUEUE_PTR	temp_queue;
	LIST_PTR			temp_list1, temp_list2;
	THREAD_PTR			wake_me_thread;

	if(timeout_queue == NULL || timeout_queue->sleep_time > timer_ticks)
		return;

	if( IsListEmpty(&(timeout_queue->queue)) )
	{
		wake_me_thread = STRUCT_ADDRESS_FROM_MEMBER(timeout_queue, THREAD, timeout_queue);
		ResumeThread(wake_me_thread);
		timeout_queue = NULL;
		return;
	}


	LIST_FOR_EACH_SAFE( temp_list1, temp_list2, (timeout_queue->queue).prev )
	{
		temp_queue = STRUCT_ADDRESS_FROM_MEMBER(temp_list1, TIMEOUT_QUEUE, queue);

		if( IsListEmpty(&(timeout_queue->queue)) )
		{
			wake_me_thread = STRUCT_ADDRESS_FROM_MEMBER(temp_queue, THREAD, timeout_queue);
			ResumeThread(wake_me_thread);
			timeout_queue = NULL;
			return;
		}

		RemoveFromList( &(temp_queue->queue) );
		wake_me_thread = STRUCT_ADDRESS_FROM_MEMBER(temp_queue, THREAD, timeout_queue);
		ResumeThread(wake_me_thread);
		/* Now the header is gone, so update it */
		timeout_queue = STRUCT_ADDRESS_FROM_MEMBER(temp_list2, TIMEOUT_QUEUE, queue);
	}
}

/*!
\brief				Searches timeout queue for an appropriate slot. This queue is sorted in ascending order aking it easy for removal.
\param	find_time	time in ticks which is the key to search in the timeout queue
returns pointer to a node in timeout queue, which has it's sleep_time just below our find_time
*/
static TIMEOUT_QUEUE_PTR SearchTimeoutQueue(INT32 find_time)
{
	TIMEOUT_QUEUE_PTR	temp_queue;
	LIST_PTR	temp_list;

	if(timeout_queue == NULL)
		return NULL;

	/* We do a hopelessly bad linear search to find the right slot. /TBD improve the search algo. */
	LIST_FOR_EACH( temp_list, &(timeout_queue->queue) )
	{
		temp_queue = STRUCT_ADDRESS_FROM_MEMBER(temp_list, TIMEOUT_QUEUE, queue);
		if(temp_queue->sleep_time <= find_time)
			continue;
		else
		{
			temp_queue = STRUCT_ADDRESS_FROM_MEMBER(temp_list->prev, TIMEOUT_QUEUE, queue);
			return temp_queue;
		}
	}
	return timeout_queue;
}


/*!
\brief				Adds a new object to the timeout queue. New object will contain thread that wants to wait for a specified time.
\param	new_object	Pointer to an object of timeout queue
*/
static void AddToTimeoutQueue(TIMEOUT_QUEUE_PTR new_object)
{
	TIMEOUT_QUEUE_PTR prev_node;
	InitList( &(new_object->queue) );

	/* Search for the correct slot in timeout_queue to place this new_object*/
	prev_node = SearchTimeoutQueue(new_object->sleep_time);
	if(prev_node == NULL)
	{
		timeout_queue = new_object;
		return;
	}
	AddToList( &(prev_node->queue), &(new_object->queue) );
}

/*!
\brief				Put this thread to sleep for the time requested.
\param	timeout		Time in milliseconds to sleep
Returns Remaining time from sleep. =0 when slept completely; >0 if interrupted during sleep; <0 If missed wakeup.
*/
INT32 Sleep(UINT32 timeout)
{
	TIMEOUT_QUEUE_PTR my_timeout_queue;

	my_timeout_queue = &(GetCurrentThread()->timeout_queue);

	my_timeout_queue->sleep_time = MILLISECONDS_TO_TICKS(timeout) + timer_ticks;

	AddToTimeoutQueue(my_timeout_queue);
	PauseThread();
	return TICKS_TO_MILLISECONDS( (my_timeout_queue->sleep_time - timer_ticks) ); 
}
