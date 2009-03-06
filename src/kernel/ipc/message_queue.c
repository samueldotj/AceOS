/*!
 * \file	kernel/ipc/message_queue.c
 * \brief	Message queue IPC handling
 */

#include <ace.h>
#include <kernel/ipc.h>
#include <kernel/pm/task.h>
#include <string.h>
#include <kernel/debug.h>

UINT32 max_message_queue_length;	/* System wide tunable to control the size of message queue in task structure */

/*! \brief						Sends a message from current task to the task specified. Holds target task's message queue lock accross the life time of function.
 * 								If flag contains NO_WAIT, then this will return with error if queue is full. By default it will wait indefinitely until it places the message on the queue.
 *  \param mbuf					Contains Message buffer.
 *  \param pid_target_task_task	Pid of target task to which the message has to be posted.
 *	\param queue_no				Offset to task->message_queue array.
 *  \param wait_time			Indicates how much time the sender can wait till the message is sent.
 */
ERROR_CODE SendMessage(MESSAGE_BUFFER_PTR mbuf, UINT32 pid_target_task, UINT8 queue_no, UINT32 wait_time)
{
	MESSAGE_BUFFER_PTR	kern_msg_buf;
	ERROR_CODE			error_ret = ERROR_SUCCESS;;
	WAIT_EVENT_PTR		my_wait_event;
	int					loop;
	TASK_PTR			to_task;

	assert(queue_no < MESSAGE_QUEUES_PER_TASK);

	to_task = PidToTask(pid_target_task);
	if(to_task == NULL)
		return ERROR_INVALID_PARAMETER;
	SpinLock( &(to_task->message_queue_lock[queue_no]) );


	if( (wait_time == MESSAGE_QUEUE_NO_WAIT) && (to_task->message_queue_length >= max_message_queue_length) )
	{
		SpinUnlock( &(to_task->lock) );
		return ERROR_NOT_ENOUGH_MEMORY;
	}
	else if( (wait_time == MESSAGE_QUEUE_CAN_WAIT) && (to_task->message_queue_length >= max_message_queue_length) )
	{
		/* Wait for this message queue to become thin */
		my_wait_event = AddToEventQueue( &(to_task->wait_event_message_queue[queue_no]) );
		SpinUnlock( &(to_task->message_queue_lock[queue_no]) ); /* We need to unlock because we block in WaitForEvent */
		WaitForEvent(my_wait_event, wait_time);
		SpinLock( &(to_task->message_queue_lock[queue_no]) ); /* take the lock again because we have returned from blocked state */
		assert(my_wait_event->fired == 1);
		kfree(my_wait_event);
	}


	kern_msg_buf = (MESSAGE_BUFFER_PTR)kmalloc(sizeof(MESSAGE_BUFFER), KMEM_NO_FAIL);
	kern_msg_buf->from_pid = GetCurrentPid();

	InitList( &(kern_msg_buf->message_buffer_queue) );

	switch( mbuf->type)
	{
		case MESSAGE_BUFFER_TYPE_VALUE:	/* In value */
				for(loop=0 ; loop < TOTAL_SYSTEM_CALL_ARGS ; loop++)
					kern_msg_buf->args[loop] = mbuf->args[loop];
				break;
		case MESSAGE_BUFFER_TYPE_BUFFER:	/* In buffers. Copy the message inside buffer from this user space to target user space */
				/* args[0] will contain start address and args[1] will contain length of the buffer. */
				if(mbuf->args[0] == NULL || mbuf->args[1] < 0 || mbuf->args[1] > PAGE_SIZE)
					return ERROR_INVALID_PARAMETER;

				kern_msg_buf->args[0] = (UINT32)kmalloc(mbuf->args[1], KMEM_NO_FAIL);
				(void)memcpy((void*)kern_msg_buf->args[0], (const void*)(mbuf->args[0]), mbuf->args[1]);
				break;
		default:
				return ERROR_INVALID_PARAMETER;
	}

	AddToListTail( &(to_task->message_queue[queue_no]->message_buffer_queue), &(kern_msg_buf->message_buffer_queue) );
	SpinUnlock( &(to_task->lock) );
	return error_ret;
}

/*! \brief				Receives a message present in the tasks's message queue.
 * 						Holds task lock accross the life time of function.
 *  \param mbuf			Pointer to an empty message buffer allocated(malloced) by user. This will be loaded with the required message. mbuf->type will be specified from user. depending on this mbuf->args[0] and mbuf->args[1] will also be filled by user.
 *  \param wait_time	Time in seconds that the user wants to wait for the message. If 0, then it's non blocking, if -1, it's blocking forever.
 */
ERROR_CODE ReceiveMessage(MESSAGE_BUFFER_PTR mbuf, UINT8 queue_no, UINT32 wait_time)
{
	MESSAGE_BUFFER_PTR	message_queue_ptr;
	ERROR_CODE			error_ret = ERROR_SUCCESS;;
	TASK_PTR			my_task;
	WAIT_EVENT_PTR		my_wait_event;
	UINT32 				ret_timeout, loop;

	if(queue_no > MESSAGE_QUEUES_PER_TASK)
		return ERROR_INVALID_PARAMETER;

	my_task = GetCurrentThread()->task;
	SpinLock( &(my_task->message_queue_lock[queue_no]) );

	message_queue_ptr = my_task->message_queue[queue_no];

	if(wait_time==MESSAGE_QUEUE_CAN_WAIT) /* Wait till the desired message arrives */
	{
		if(message_queue_ptr == NULL)
		{
			SpinUnlock( &(my_task->message_queue_lock[queue_no]) );
			my_wait_event = AddToEventQueue( &(my_task->wait_event_message_queue[queue_no]) );
			WaitForEvent(my_wait_event, 0); /* block forever until some message arrives */
			SpinLock( &(my_task->message_queue_lock[queue_no]) ); /* take the lock after returning from blocked state */
		}
	}
	else if(wait_time==MESSAGE_QUEUE_NO_WAIT)
	{
		if(message_queue_ptr == NULL)
		{
			SpinUnlock( &(my_task->message_queue_lock[queue_no]) );
			return ERROR_NOT_FOUND;
		}
	}
	else /* wait_time > 0 */
	{
		search_queue:
			if(message_queue_ptr == NULL)
			{
				if(wait_time < 0)
				{
					SpinUnlock( &(my_task->message_queue_lock[queue_no]) );
					return ERROR_NOT_FOUND;	/* In case of missed wakeup's from WaitForEvent, wait time will be in -ve */
				}

				my_wait_event = AddToEventQueue( &(my_task->wait_event_message_queue[queue_no]) );
				SpinUnlock( &(my_task->message_queue_lock[queue_no]) );
				ret_timeout = WaitForEvent(my_wait_event, wait_time); /* Block till wait_time expires or until any message arrives */
				if(ret_timeout == 0 || my_task->wait_event_message_queue[queue_no] == NULL)	/* timeout happened and our event didn't get fired */
					return ERROR_NOT_FOUND;
				else if(wait_time-ret_timeout > 0)	/* ret_timeout is > 0 and < wait_time and indicates time left in sleep */
				{
					assert(my_wait_event->fired == 1);
					/* we got here because an even got fired before the timeout happened */
					SpinLock( &(my_task->message_queue_lock[queue_no]) );
					message_queue_ptr = my_task->message_queue[queue_no];
					wait_time -= ret_timeout;
					goto search_queue;
				}
				/* we have got the message, so can continue fetching it */
				SpinLock( &(my_task->message_queue_lock[queue_no]) ); /* take the lock after returning from blocked state */
			}
	}
	
	RemoveFromList( &(message_queue_ptr->message_buffer_queue) );

	switch(mbuf->type)
	{
		case MESSAGE_BUFFER_TYPE_VALUE:	/* In value */
				for(loop=0 ; loop < TOTAL_SYSTEM_CALL_ARGS ; loop++)
					mbuf->args[loop] = message_queue_ptr->args[loop];
				break;
		case MESSAGE_BUFFER_TYPE_BUFFER:	/* In buffers. Copy the message inside buffer from this user space to target user space */
				(void)memcpy((void*)(mbuf->args[0]), (const void*)message_queue_ptr->args[0], mbuf->args[1]);
				break;
	}

	SpinUnlock( &(my_task->message_queue_lock[queue_no]) );
	return error_ret;
}
