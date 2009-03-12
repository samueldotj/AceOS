/*!
 * \file	kernel/ipc/message_queue.c
 * \brief	Message queue IPC handling
 */

#include <ace.h>
#include <kernel/ipc.h>
#include <kernel/pm/task.h>
#include <string.h>
#include <kernel/debug.h>

UINT32 max_message_queue_length;	/* System wide tunable to control the size of message queue in message_queue structure */

/*! \brief					Wrapper to SendMessageCore.
 *  \param type				Indicates message type as ByValue or ByReference
 *  \param arg1				Pointer to data buffer in user space of size atleast *arg2 or arg1(ByValue).
 *  \param arg2				Length of message that needs to be copied or arg2(ByValue).
 *  \param arg3				Store to hold optional arg1 or arg3(ByValue)
 *  \param arg4				Store to hold optional arg2 or arg4(ByValue)
 *  \param pid_target_task	Pid of target task to which the message has to be posted.
 *	\param queue_no			Offset to task->message_queue array.
 *  \param wait_time		Indicates how much time the sender can wait till the message is sent.
 */
ERROR_CODE SendMessage(MESSAGE_TYPE type, register unsigned int arg1, register unsigned int arg2, register unsigned int arg3, register unsigned int arg4, UINT32 pid_target_task, UINT8 queue_no, UINT32 wait_time)
{
	TASK_PTR			to_task;

	assert(queue_no < MESSAGE_QUEUES_PER_TASK);

	to_task = PidToTask(pid_target_task);
	if(to_task == NULL)
		return ERROR_INVALID_PARAMETER;

	return SendMessageCore(type, arg1, arg2, arg3, arg4, &(to_task->message_queue[queue_no]), wait_time);
}

/*! \brief					Sends a message from current task to the message queue specified. Holds target message queue's lock accross the life time of function.
 * 							If wait_time contains NO_WAIT, then this will return with error if queue is full. By default it will wait indefinitely until it places the message on the queue.
 *  \param type				Indicates message type as ByValue or ByReference
 *  \param arg1				Pointer to data buffer in user space of size atleast *arg2.
 *  \param arg2				Length of message that needs to be copied.
 *  \param arg3				Store to hold optional argument1
 *  \param arg4				Store to hold optional argument2
 								
 *	\param message_queue	Pointer to target message queue, where the message has to be delivered.
 *  \param wait_time		Indicates how much time the sender can wait till the message is sent.
 */
ERROR_CODE SendMessageCore(MESSAGE_TYPE type, register unsigned int arg1, register unsigned int arg2, register unsigned int arg3, register unsigned int arg4, MESSAGE_QUEUE_PTR message_queue, UINT32 wait_time)
{
	MESSAGE_BUFFER_PTR	kern_msg_buf;
	ERROR_CODE			error_ret = ERROR_SUCCESS;;
	WAIT_EVENT_PTR		my_wait_event;
	UINT32				ret_time;
	WAIT_EVENT_PTR		wait_queue;
	SPIN_LOCK_PTR		wait_lock;

	if(message_queue == NULL)
		return ERROR_INVALID_PARAMETER;

	SpinLock( &(message_queue->lock) );

	/*Sending this message depends on queue length and wait time */
	if(message_queue->buf_count >= max_message_queue_length)
	{
		if( wait_time == MESSAGE_QUEUE_NO_WAIT )
		{
			SpinUnlock( &(message_queue->lock) );
			return ERROR_NOT_ENOUGH_MEMORY;
		}
		else if( wait_time != MESSAGE_QUEUE_NO_WAIT )
		{
			wait_queue = message_queue->wait_event_msg_queue;
			wait_lock = &(message_queue->wait_event_msg_queue_lock);

			/* Wait for this message queue to become thin */
			SpinLock( wait_lock );
			my_wait_event = AddToEventQueue( &wait_queue );
			SpinUnlock( wait_lock );

			if(wait_time == MESSAGE_QUEUE_CAN_WAIT)
				wait_time = 0;	

			SpinUnlock( &(message_queue->lock) );
			ret_time = WaitForEvent(my_wait_event, wait_time);
			SpinLock( &(message_queue->lock) ); /* take the lock again because we have returned from blocked state */

			if(ret_time == 0) /* no event fired and we timeout out */
			{
				SpinUnlock( &(message_queue->lock) );
				return ERROR_NOT_ENOUGH_MEMORY;
			}

			assert(my_wait_event->fired == 1);

			SpinLock( wait_lock );
			RemoveEventFromQueue( my_wait_event, &wait_queue);
			SpinUnlock( wait_lock );

			kfree(my_wait_event);
		}
	}
	kern_msg_buf = (MESSAGE_BUFFER_PTR)kmalloc(sizeof(MESSAGE_BUFFER), KMEM_NO_FAIL);

	InitList( &(kern_msg_buf->message_buffer_queue) );
	kern_msg_buf->type = type;

	switch(type)
	{
		case MESSAGE_TYPE_VALUE:	/* In value */
				kern_msg_buf->args[0] = arg1;
				kern_msg_buf->args[1] = arg2;
				kern_msg_buf->args[2] = arg3;
				kern_msg_buf->args[3] = arg4;
				break;
		case MESSAGE_TYPE_REFERENCE:	/* In buffers. Copy the message inside buffer from this user space to target user space */
				/* args[0] will contain start address and args[1] will contain length of the buffer. args[2] and args[3] contain optional storage for values */
				if(arg1 == NULL || arg2 < 0 || arg2 > PAGE_SIZE)
					return ERROR_INVALID_PARAMETER;


				kern_msg_buf->args[ARG_ADDRESS_INDEX] = (UINT32)kmalloc(arg2, KMEM_NO_FAIL);
				(void)memcpy((void*)kern_msg_buf->args[ARG_ADDRESS_INDEX], (const void*)(arg1), arg2);
				kern_msg_buf->args[ARG_LENGTH_INDEX] = arg2;

				/* Copy optional values in last 2 arguments */
				kern_msg_buf->args[ARG1_INDEX] = arg3;
				kern_msg_buf->args[ARG2_INDEX] = arg4;
				break;
		default:
				return ERROR_INVALID_PARAMETER;
	}

	if(message_queue->msg_queue == NULL)
		message_queue->msg_queue = kern_msg_buf;
	else
		AddToListTail( &(message_queue->msg_queue->message_buffer_queue), &(kern_msg_buf->message_buffer_queue) );

	message_queue->buf_count++;
	SpinUnlock( &(message_queue->lock) );
	return error_ret;
}

/*! \brief				Receives a message present in the tasks's message queue.
 * 						Holds task lock accross the life time of function.
 *  \param type			Indicates message type as ByValue or ByReference
 *  \param arg1			Pointer to data buffer in user space of size atleast *arg2.
 *  \param arg2			Length of message that needs to be copied.
 *  \param arg3			Store to hold optional argument1
 *  \param arg4			Store to hold optional argument2
 *	\param queue_no		This is the index into message_queue array in task structure.
 *  \param wait_time	Time in seconds that the user wants to wait for the message. If 0, then it's non blocking, if -1, it's blocking forever.
 */
ERROR_CODE ReceiveMessage(MESSAGE_TYPE type, unsigned int *arg1, unsigned int *arg2, unsigned int *arg3, unsigned int *arg4, UINT8 queue_no, UINT32 wait_time)
{
	TASK_PTR			my_task;

	if(queue_no > MESSAGE_QUEUES_PER_TASK)
		return ERROR_INVALID_PARAMETER;

	my_task = GetCurrentThread()->task;
	return ReceiveMessageCore(type, arg1, arg2, arg3, arg4, &(my_task->message_queue[queue_no]), wait_time);
}


/*! \brief					Receives a message present in the tasks's message queue.
 * 							Holds task lock accross the life time of function.
 *  \param type				Indicates message type as ByValue or ByReference
 *  \param arg1				Pointer to data buffer in user space of size atleast *arg2.
 *  \param arg2				Length of message that needs to be copied.
 *  \param arg3				Store to hold optional argument1
 *  \param arg4				Store to hold optional argument2
 *	\param message_queue	Pointer to message queue from where message has to be fetched
 *  \param wait_time		Time in seconds that the user wants to wait for the message. If 0, then it's non blocking, if -1, it's blocking forever.
 */
ERROR_CODE ReceiveMessageCore(MESSAGE_TYPE type, unsigned int *arg1, unsigned int *arg2, unsigned int *arg3, unsigned int *arg4, MESSAGE_QUEUE_PTR message_queue, UINT32 wait_time)
{
	WAIT_EVENT_PTR		my_wait_event, wait_queue;
	UINT32				ret_time;
	MESSAGE_BUFFER_PTR	rem_buf;
	SPIN_LOCK_PTR wait_lock;

	if(message_queue == NULL)
		return ERROR_INVALID_PARAMETER;

	SpinLock( &(message_queue->lock) );

	if(message_queue->buf_count == 0)
	{
		if(wait_time == MESSAGE_QUEUE_NO_WAIT)
			return ERROR_NOT_FOUND;
		else /* Wait till the desired message arrives for time specified in wait_time */
		{
			wait_queue = message_queue->wait_event_msg_queue;
			wait_lock = &(message_queue->wait_event_msg_queue_lock);

			SpinLock( wait_lock );
			my_wait_event = AddToEventQueue( &wait_queue );
			SpinUnlock( wait_lock );

			if(wait_time == MESSAGE_QUEUE_CAN_WAIT)
				wait_time = 0;	

			SpinUnlock( &(message_queue->lock) );
			ret_time = WaitForEvent(my_wait_event, wait_time); /* Block till wait_time expires or until timer expires */
			SpinLock( &(message_queue->lock) );

			if(ret_time == 0)	/* timeout happened and our event didn't get fired */
			{
				SpinUnlock( &(message_queue->lock) );
				return ERROR_NOT_FOUND;
			}

			assert(my_wait_event->fired == 1 && message_queue->buf_count > 0);

			SpinLock( wait_lock);
			RemoveEventFromQueue(my_wait_event, &wait_queue);
			SpinUnlock( wait_lock);
			/* we have got the message, so can continue fetching it */
		}
	}

	rem_buf = message_queue->msg_queue;
	message_queue->buf_count--;

	if(message_queue->buf_count == 0)
		message_queue->msg_queue = NULL;
	else
		message_queue->msg_queue = STRUCT_ADDRESS_FROM_MEMBER((rem_buf->message_buffer_queue).next, MESSAGE_BUFFER, message_buffer_queue);
	
	RemoveFromList( &(rem_buf->message_buffer_queue) );

	switch(type)
	{
		case MESSAGE_TYPE_SKIP:
			break;
		case MESSAGE_TYPE_VALUE:	/* In value */
				if(arg1 == NULL || arg2 == NULL || arg3 == NULL || arg4 == NULL)
					return ERROR_INVALID_PARAMETER;

				*arg1 = rem_buf->args[0];
				*arg2 = rem_buf->args[1];
				*arg3 = rem_buf->args[2];
				*arg4 = rem_buf->args[3];
				break;
		case MESSAGE_TYPE_REFERENCE:	/* In buffers. Copy the message inside buffer from this user space to target user space */
				if(arg1 == NULL || arg2 == NULL || arg3 == NULL || arg4 == NULL || (arg2!=NULL && *arg2 < 0) )
					return ERROR_INVALID_PARAMETER;

				(void)memcpy((void*)(*arg1), (const void*)rem_buf->args[ARG_ADDRESS_INDEX], *arg2);
				/* Copy values in last 2 arguments */
				*arg3 = rem_buf->args[ARG1_INDEX];
				*arg4 = rem_buf->args[ARG2_INDEX];
				break;
	}

	kfree(rem_buf);
	SpinUnlock( &(message_queue->lock) );
	return ERROR_SUCCESS;
}

UINT32 GetTotalMessageCount(MESSAGE_QUEUE_PTR msg_queue)
{
	return msg_queue->buf_count;
}

void SkipMessage(MESSAGE_QUEUE_PTR msg_queue)
{
	ReceiveMessageCore(MESSAGE_TYPE_SKIP, NULL, NULL, NULL, NULL, msg_queue, MESSAGE_QUEUE_NO_WAIT);
}


ERROR_CODE GetNextMessageInfo(UINT32 *type, UINT32 *length, MESSAGE_QUEUE_PTR msg_queue)
{
	MESSAGE_BUFFER_PTR my_buf;

	if(msg_queue == NULL)
		return ERROR_INVALID_PARAMETER;

	SpinLock( &(msg_queue->lock) );

	if(msg_queue->buf_count <= 1)
	{
		SpinUnlock( &(msg_queue->lock) );
		return ERROR_NOT_FOUND;
	}
	
	my_buf = STRUCT_ADDRESS_FROM_MEMBER( (msg_queue->msg_queue->message_buffer_queue).next, MESSAGE_BUFFER, message_buffer_queue );
	if(my_buf == msg_queue->msg_queue)
	{
		kprintf("message_queue=%p buf_count=%d\n", msg_queue, msg_queue->buf_count);
		SpinUnlock( &(msg_queue->lock) );
		panic("GetNextMessageInfo: message queue garbled\n");
	}

	*type = my_buf->type;
	*length = my_buf->args[3];
	return ERROR_SUCCESS;
}
