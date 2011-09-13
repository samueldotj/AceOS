/*!
 * \file	kernel/ipc/message_queue.c
 * \brief	Message queue IPC handling
 * \todo 	Somebody has to release the memory shared by MESSAGE_TYPE_SHARE and MESSAGE_TYPE_SHARE_PA after sometime
 */

#include <ace.h>
#include <string.h>
#include <kernel/ipc.h>
#include <kernel/debug.h>
#include <kernel/pm/pm_types.h>
#include <kernel/pm/task.h>
#include <kernel/pm/thread.h>

/*! System wide tunable to control the size of message queue in message_queue structure */
UINT32 max_message_queue_length=100;	

static void RemoveFromMessageQueue(MESSAGE_QUEUE_PTR message_queue);
static void AddToMessageQueue(MESSAGE_QUEUE_PTR message_queue, MESSAGE_BUFFER_PTR buf);
static inline ERROR_CODE MessageBufferToArgs(MESSAGE_BUFFER_PTR msg_buf, IPC_ARG_TYPE arg1, IPC_ARG_TYPE arg2, IPC_ARG_TYPE arg3, IPC_ARG_TYPE arg4, IPC_ARG_TYPE arg5, IPC_ARG_TYPE arg6);
static ERROR_CODE WaitOnMessageQueue(MESSAGE_QUEUE_PTR message_queue, int wait_time, MESSAGE_TYPE * type);
static inline ERROR_CODE ArgsToMessageBuffer(TASK_PTR target_task, MESSAGE_BUFFER_PTR msg_buf, MESSAGE_TYPE type, IPC_ARG_TYPE arg1, IPC_ARG_TYPE arg2, IPC_ARG_TYPE arg3, IPC_ARG_TYPE arg4, IPC_ARG_TYPE arg5, IPC_ARG_TYPE arg6);

/*! \brief					Initializes the given message queue
 *	\param	message_queue	Message queue to be initialized
 */
void InitMessageQueue(MESSAGE_QUEUE_PTR message_queue)
{
	memset( message_queue, 0, sizeof(MESSAGE_QUEUE) );
	InitSpinLock( &message_queue->lock );
	InitSpinLock( &message_queue->wait_event_msg_queue_lock );
}

/*! \brief					Wrapper to SendMessageCore.
 *  \param pid_target_task	Pid of target task to which the message has to be posted.
 *	\param queue_no			Offset to task->message_queue array.
  *	\param type 			Type of the message passed.
 *	\param arg1				Argument 1 of the message
 *	\param arg2 			Argument 2 of the message
 *	\param arg3				Argument 3 of the message
 *	\param arg4 			Argument 4 of the message
  *	\param arg5				Argument 5 of the message(Address to copy for MESSAGE_TYPE_REFERENCE, MESSAGE_TYPE_SHARE and MESSAGE_TYPE_SHARE_PA)
 *	\param arg6 			Argument 6 of the message(Length of the buffer for MESSAGE_TYPE_REFERENCE, MESSAGE_TYPE_SHARE and MESSAGE_TYPE_SHARE_PA)
 *  \param wait_time		Indicates how much time the sender can wait till the message is sent.
 */
ERROR_CODE SendMessage(UINT32 pid_target_task, UINT8 queue_no, MESSAGE_TYPE type, IPC_ARG_TYPE arg1, IPC_ARG_TYPE arg2, IPC_ARG_TYPE arg3, IPC_ARG_TYPE arg4, IPC_ARG_TYPE arg5, IPC_ARG_TYPE arg6, UINT32 wait_time)
{
	TASK_PTR to_task;

	assert(queue_no < MESSAGE_QUEUES_PER_TASK);

	to_task = PidToTask(pid_target_task);
	if(to_task == NULL)
		return ERROR_INVALID_PARAMETER;

	return SendMessageCore(to_task, &to_task->message_queue[queue_no], type, arg1, arg2, arg3, arg4, arg5, arg6, wait_time);
}

/*! \brief					Sends a message from current task to the message queue specified. Holds target message queue's lock accross the life time of function.
 * 							If wait_time contains NO_WAIT, then this will return with error if queue is full. By default it will wait indefinitely until it places the message on the queue.
 *	\param message_queue	Pointer to target message queue, where the message has to be delivered.
 *	\param type 			Type of the message passed.
 *	\param arg1				Argument 1 of the message
 *	\param arg2 			Argument 2 of the message
 *	\param arg3				Argument 3 of the message
 *	\param arg4 			Argument 4 of the message
 *	\param arg5				Argument 5 of the message(Address to copy for MESSAGE_TYPE_REFERENCE, MESSAGE_TYPE_SHARE and MESSAGE_TYPE_SHARE_PA)
 *	\param arg6 			Argument 6 of the message(Length of the buffer for MESSAGE_TYPE_REFERENCE, MESSAGE_TYPE_SHARE and MESSAGE_TYPE_SHARE_PA)
 *  \param wait_time		Indicates how much time the sender can wait till the message is sent.
 */
ERROR_CODE SendMessageCore(TASK_PTR target_task, MESSAGE_QUEUE_PTR message_queue, MESSAGE_TYPE type, IPC_ARG_TYPE arg1, IPC_ARG_TYPE arg2, IPC_ARG_TYPE arg3, IPC_ARG_TYPE arg4, IPC_ARG_TYPE arg5, IPC_ARG_TYPE arg6, UINT32 wait_time)
{
	MESSAGE_BUFFER_PTR msg_buf;
	ERROR_CODE	error_ret = ERROR_SUCCESS;;

	if(message_queue == NULL)
		return ERROR_INVALID_PARAMETER;

	SpinLock( &(message_queue->lock) );

	/*Sending this message depends on queue length and wait time */
	if(message_queue->buf_count >= max_message_queue_length)
	{
		if( wait_time == MESSAGE_QUEUE_NO_WAIT )
			error_ret = ERROR_NOT_ENOUGH_MEMORY;
		else
			error_ret = WaitOnMessageQueue( message_queue, wait_time, NULL );
		if ( error_ret != ERROR_SUCCESS )
		{
			error_ret = ERROR_NOT_ENOUGH_MEMORY;
			goto done;
		}
	}
	msg_buf = (MESSAGE_BUFFER_PTR)kmalloc(sizeof(MESSAGE_BUFFER), 0);
	if ( msg_buf == NULL )
	{
		error_ret = ERROR_NOT_ENOUGH_MEMORY;
		goto done;
	}

	ArgsToMessageBuffer(target_task, msg_buf, type, arg1, arg2, arg3, arg4, arg5, arg6 );
	AddToMessageQueue( message_queue, msg_buf );

done:
	SpinUnlock( &(message_queue->lock) );
	return error_ret;
}

/*! \brief				Receives a message present in the tasks's message queue.
 * 						Holds task lock accross the life time of function.
 *	\param queue_no		This is the index into message_queue array in task structure.
 *	\param type 		Type of the message passed.
 *	\param arg1			Holder to put argument 1 of the message
 *	\param arg2 		Holder to put argument 2 of the message
 *	\param arg3			Holder to put argument 3 of the message
 *	\param arg4 		Holder to put argument 4 of the message
  *	\param arg5			Holder to put argument 5 of the message(Address for MESSAGE_TYPE_REFERENCE, MESSAGE_TYPE_SHARE and MESSAGE_TYPE_SHARE_PA)
 *	\param arg6 		Holder to put argument 6 of the message(Length of the buffer for MESSAGE_TYPE_REFERENCE, MESSAGE_TYPE_SHARE and MESSAGE_TYPE_SHARE_PA)
 *  \param wait_time	Time in seconds that the user wants to wait for the message. If 0, then it's non blocking, if -1, it's blocking forever.
 */
ERROR_CODE ReceiveMessage(UINT8 queue_no, IPC_ARG_TYPE arg1, IPC_ARG_TYPE arg2, IPC_ARG_TYPE arg3, IPC_ARG_TYPE arg4, IPC_ARG_TYPE arg5, IPC_ARG_TYPE arg6, UINT32 wait_time)
{
	TASK_PTR			my_task;

	if(queue_no > MESSAGE_QUEUES_PER_TASK)
		return ERROR_INVALID_PARAMETER;

	my_task = GetCurrentThread()->task;
	return ReceiveMessageCore(&my_task->message_queue[queue_no], arg1, arg2, arg3, arg4, arg5, arg6, wait_time);
}

/*! \brief					Receives a message present in the tasks's message queue.
 * 							Holds task lock accross the life time of function.
 *	\param message_queue	Pointer to message queue from where message has to be fetched
 *	\param type 			Type of the message passed.
 *	\param arg1				Holder to put argument 1 of the message
 *	\param arg2 			Holder to put argument 2 of the message
 *	\param arg3				Holder to put argument 3 of the message
 *	\param arg4 			Holder to put argument 4 of the message
  *	\param arg5				Holder to put argument 5 of the message(Address for MESSAGE_TYPE_REFERENCE, MESSAGE_TYPE_SHARE and MESSAGE_TYPE_SHARE_PA)
 *	\param arg6 			Holder to put argument 6 of the message(Length of the buffer for MESSAGE_TYPE_REFERENCE, MESSAGE_TYPE_SHARE and MESSAGE_TYPE_SHARE_PA)
 *  \param wait_time		Time in seconds that the user wants to wait for the message. If 0, then it's non blocking, if -1, it's blocking forever.
 */
ERROR_CODE ReceiveMessageCore(MESSAGE_QUEUE_PTR message_queue, IPC_ARG_TYPE arg1, IPC_ARG_TYPE arg2, IPC_ARG_TYPE arg3, IPC_ARG_TYPE arg4, IPC_ARG_TYPE arg5, IPC_ARG_TYPE arg6, UINT32 wait_time)
{
	ERROR_CODE ret = ERROR_SUCCESS;
	
	assert(message_queue != NULL);

	SpinLock( &(message_queue->lock) );
	if(message_queue->buf_count == 0)
	{
		if( wait_time == MESSAGE_QUEUE_NO_WAIT )
			ret = ERROR_NOT_FOUND;
		else
		{
			SpinUnlock( &(message_queue->lock) );
			ret = WaitOnMessageQueue( message_queue, wait_time, NULL );
			SpinLock( &(message_queue->lock) );
		}
	}
	if ( ret != ERROR_SUCCESS || message_queue->buf_count == 0)
	{
		ret = ERROR_NOT_FOUND;
		goto done;
	}

	/*fill the args*/
	ret = MessageBufferToArgs( message_queue->msg_queue, arg1, arg2, arg3, arg4, arg5, arg6 );
	if( ret != ERROR_SUCCESS )
		goto done;
	
	RemoveFromMessageQueue(message_queue);
		
done:
	SpinUnlock( &(message_queue->lock) );
	return ret;
}

/*! Replies to the last received message
 *	\param type 	Type of the message passed.
 *	\param arg1				Argument 1 of the message
 *	\param arg2 			Argument 2 of the message
 *	\param arg3				Argument 3 of the message
 *	\param arg4 			Argument 4 of the message
 *	\param arg5				Argument 5 of the message(Address to copy for MESSAGE_TYPE_REFERENCE, MESSAGE_TYPE_SHARE and MESSAGE_TYPE_SHARE_PA)
 *	\param arg6 			Argument 6 of the message(Length of the buffer for MESSAGE_TYPE_REFERENCE, MESSAGE_TYPE_SHARE and MESSAGE_TYPE_SHARE_PA)
 */
ERROR_CODE ReplyToLastMessage(MESSAGE_TYPE type, IPC_ARG_TYPE arg1, IPC_ARG_TYPE arg2, IPC_ARG_TYPE arg3, IPC_ARG_TYPE arg4, IPC_ARG_TYPE arg5, IPC_ARG_TYPE arg6)
{
	THREAD_PTR current_thread, to_thread;
	ERROR_CODE ret;
	
	current_thread = GetCurrentThread();
	to_thread = current_thread->ipc_reply_to_thread;
	if ( to_thread == NULL )
	{
		kprintf("Thread not found");
		return ERROR_NOT_FOUND;
	}
			
	/*fill the message*/
	ret = ArgsToMessageBuffer( to_thread->task, &to_thread->ipc_reply_message, type, arg1, arg2, arg3, arg4, arg5, arg6 );
	if ( ret != ERROR_SUCCESS )
		return ret;
	
	/*wakeup the thread*/
	WakeUpEvent( &to_thread->ipc_reply_event, WAIT_EVENT_WAKE_UP_ALL );
	
	current_thread->ipc_reply_to_thread = NULL;
	
	return ERROR_SUCCESS;
}
/*! Waits for a reply for the last send message
 *	\param type 	Type of the message to receive.
 *	\param arg1				Argument 1 of the message
 *	\param arg2 			Argument 2 of the message
 *	\param arg3				Argument 3 of the message
 *	\param arg4 			Argument 4 of the message
 *	\param arg5				Argument 5 of the message(Address to copy for MESSAGE_TYPE_REFERENCE, MESSAGE_TYPE_SHARE and MESSAGE_TYPE_SHARE_PA)
 *	\param arg6 			Argument 6 of the message(Length of the buffer for MESSAGE_TYPE_REFERENCE, MESSAGE_TYPE_SHARE and MESSAGE_TYPE_SHARE_PA)
 *  \param timeout	Max time to wait for the reply
 */
ERROR_CODE WaitForReply(MESSAGE_TYPE type, IPC_ARG_TYPE arg1, IPC_ARG_TYPE arg2, IPC_ARG_TYPE arg3, IPC_ARG_TYPE arg4, IPC_ARG_TYPE arg5, IPC_ARG_TYPE arg6, int timeout)
{
	THREAD_PTR current_thread;
	WAIT_EVENT_PTR wait_event;
	
	current_thread = GetCurrentThread();
	
	/*wait for reply with timeout*/
	wait_event = AddToEventQueue( &current_thread->ipc_reply_event );
	if ( WaitForEvent(wait_event, timeout ) == 0 )
		return ERROR_TIMEOUT;

	if ( current_thread->ipc_reply_message.type != type )
		return ERROR_INVALID_FORMAT;
		
	/*we got our reply*/
	return MessageBufferToArgs( &current_thread->ipc_reply_message, arg1, arg2, arg3, arg4, arg5, arg6 );
}

/*! Discards the first message in the given message queue
*/
void SkipMessage(MESSAGE_QUEUE_PTR msg_queue)
{
	ReceiveMessageCore(NULL, NULL, NULL, NULL, NULL, NULL, msg_queue, MESSAGE_QUEUE_NO_WAIT);
}

/*! \brief					Returns information about next message present in the message queue.
 *	\param message_queue	Pointer to message queue from where message has to be fetched
 *	\param type 			Type of the message passed.
 *	\param length 			Length of the message buffer
 *  \param wait_time		Time in seconds that the user wants to wait for the message. If 0, then it's non blocking, if -1, it's blocking forever.
 */
ERROR_CODE GetNextMessageInfo(MESSAGE_QUEUE_PTR message_queue, MESSAGE_TYPE *type, UINT32 *length, int wait_time)
{
	ERROR_CODE ret = ERROR_SUCCESS;
	MESSAGE_BUFFER_PTR mb = NULL;
	
	assert(message_queue != NULL);

	SpinLock( &(message_queue->lock) );
	if(message_queue->buf_count == 0)
	{
		if( wait_time == MESSAGE_QUEUE_NO_WAIT )
			ret = ERROR_NOT_FOUND;
		else
		{
			SpinUnlock( &(message_queue->lock) );
			ret = WaitOnMessageQueue( message_queue, wait_time, NULL );
			SpinLock( &(message_queue->lock) );
		}
			
	}
	/*wait might timedout or somebody might have taken message before we took the spin lock*/
	if( ret!=ERROR_SUCCESS || message_queue->buf_count == 0)
	{
		ret = ERROR_NOT_FOUND;
		goto done;
	}

	mb = message_queue->msg_queue;
	*type = mb->type;
	if ( mb->type == MESSAGE_TYPE_REFERENCE )
		*length = (int)mb->args[IPC_LENGTH_ARG_INDEX];

done:
	SpinUnlock( &(message_queue->lock) );
	return ret;
}

/*! Copies values from function arguments to message buffer structure
 *	\param target_task	Receiver task
 *	\param msg_buf		Message buffer to which arguments to copy.
 *	\param type 		Type of the message passed.
 *	\param arg1			Argument 1 of the message
 *	\param arg2 		Argument 2 of the message
 *	\param arg3			Argument 3 of the message
 *	\param arg4 		Argument 4 of the message
 *	\param arg5			Argument 5 of the message(Address to copy for MESSAGE_TYPE_REFERENCE, MESSAGE_TYPE_SHARE and MESSAGE_TYPE_SHARE_PA)
 *	\param arg6 		Argument 6 of the message(Length of the buffer for MESSAGE_TYPE_REFERENCE, MESSAGE_TYPE_SHARE and MESSAGE_TYPE_SHARE_PA)
 */
static inline ERROR_CODE ArgsToMessageBuffer(TASK_PTR target_task, MESSAGE_BUFFER_PTR msg_buf, MESSAGE_TYPE type, IPC_ARG_TYPE arg1, IPC_ARG_TYPE arg2, IPC_ARG_TYPE arg3, IPC_ARG_TYPE arg4, IPC_ARG_TYPE arg5, IPC_ARG_TYPE arg6)
{
	ERROR_CODE ret;
	VADDR copy_va;
	
	msg_buf->args[IPC_ARG_INDEX_1] = arg1;
	msg_buf->args[IPC_ARG_INDEX_2] = arg2;
	msg_buf->args[IPC_ARG_INDEX_3] = arg3;
	msg_buf->args[IPC_ARG_INDEX_4] = arg4;
	switch(type)
	{
		/*copy all the arguments to the message*/
		case MESSAGE_TYPE_VALUE:
			msg_buf->args[IPC_ARG_INDEX_5] = arg5;
			msg_buf->args[IPC_ARG_INDEX_6] = arg6;
			break;
		/*Allocate kernel buffer of length argument 5 and copy the contents pointed by argument 6 to it*/
		case MESSAGE_TYPE_REFERENCE:
			if(IPR_ARGUMENT_ADDRESS == NULL || IPC_ARGUMENT_LENGTH < 0 || IPC_ARGUMENT_LENGTH > PAGE_SIZE)
				return ERROR_INVALID_PARAMETER;

			msg_buf->args[IPC_ADDRESS_ARG_INDEX] = kmalloc(IPC_ARGUMENT_LENGTH, 0);
			if ( msg_buf->args[IPC_ADDRESS_ARG_INDEX] == NULL )
				return ERROR_NOT_ENOUGH_MEMORY;

			memcpy(msg_buf->args[IPC_ADDRESS_ARG_INDEX], IPR_ARGUMENT_ADDRESS, IPC_ARGUMENT_LENGTH);
			msg_buf->args[IPC_LENGTH_ARG_INDEX] = (IPC_ARG_TYPE)IPC_ARGUMENT_LENGTH;

			break;
		/*share the virtual address pointed by arg5 of length arg6 to target task*/
		case MESSAGE_TYPE_SHARE:
			if(IPR_ARGUMENT_ADDRESS == NULL || !IS_PAGE_ALIGNED((long)IPR_ARGUMENT_ADDRESS) || IPC_ARGUMENT_LENGTH <= 0 || !IS_PAGE_ALIGNED(IPC_ARGUMENT_LENGTH))
				return ERROR_INVALID_PARAMETER;
			copy_va = (VADDR)IPR_ARGUMENT_ADDRESS;
			ret = CopyVirtualAddressRange(GetCurrentVirtualMap(), (VADDR)IPR_ARGUMENT_ADDRESS, IPC_ARGUMENT_LENGTH, target_task->virtual_map, &copy_va, IPC_ARGUMENT_LENGTH, PROT_READ, VM_UNIT_FLAG_PRIVATE);
			if( ret != ERROR_SUCCESS )
				return ret;
			
			/*copy the result*/
			msg_buf->args[IPC_ARG_INDEX_5] = (IPC_ARG_TYPE)copy_va;
			msg_buf->args[IPC_ARG_INDEX_6] = (IPC_ARG_TYPE)IPC_ARGUMENT_LENGTH;
			
			type = MESSAGE_TYPE_REFERENCE;
			break;
		/*internal message to share the virtual page*/
		case MESSAGE_TYPE_SHARE_PA:
			assert( IPR_ARGUMENT_ADDRESS != NULL && IPC_ARGUMENT_LENGTH == PAGE_SIZE);
			copy_va = MapPhysicalMemory(target_task->virtual_map, (VADDR)IPR_ARGUMENT_ADDRESS, IPC_ARGUMENT_LENGTH, 0, PROT_READ | PROT_WRITE);
			if ( copy_va == NULL )
				return NULL;
			msg_buf->args[IPC_ARG_INDEX_5] = (IPC_ARG_TYPE)copy_va;
			msg_buf->args[IPC_ARG_INDEX_6] = (IPC_ARG_TYPE)IPC_ARGUMENT_LENGTH;
			
			type = MESSAGE_TYPE_VALUE;
			break;
		default:
			return ERROR_INVALID_PARAMETER;
	}
	msg_buf->type = type;
	msg_buf->sender_thread = GetCurrentThread();

	return ERROR_SUCCESS;
}

/*! Copies values from message buffer structure to function arguments
 *	\param msg_buf 		Message buffer from which arguments to copy.
 *	\param type 		Type of the message passed.
 *	\param arg1			Argument 1 of the message
 *	\param arg2 		Argument 2 of the message
 *	\param arg3			Argument 3 of the message
 *	\param arg4 		Argument 4 of the message
 *	\param arg5			Argument 5 of the message(Address to copy for MESSAGE_TYPE_REFERENCE, MESSAGE_TYPE_SHARE and MESSAGE_TYPE_SHARE_PA)
 *	\param arg6 		Argument 6 of the message(Length of the buffer for MESSAGE_TYPE_REFERENCE, MESSAGE_TYPE_SHARE and MESSAGE_TYPE_SHARE_PA)
 */
static inline ERROR_CODE MessageBufferToArgs(MESSAGE_BUFFER_PTR msg_buf, IPC_ARG_TYPE arg1, IPC_ARG_TYPE arg2, IPC_ARG_TYPE arg3, IPC_ARG_TYPE arg4, IPC_ARG_TYPE arg5, IPC_ARG_TYPE arg6)
{
	THREAD_PTR current_thread;
	int copy_size;
	
	assert( msg_buf != NULL );
	
	/*! if the caller really wants to skip the message then do it*/
	if ( arg1 == NULL )
		goto done;
	
	if ( arg1 )
		*(long **)arg1 = msg_buf->args[IPC_ARG_INDEX_1];
	if ( arg2 )
		*(long **)arg2 = msg_buf->args[IPC_ARG_INDEX_2];
	if ( arg3 )
		*(long **)arg3 = msg_buf->args[IPC_ARG_INDEX_3];
	if ( arg4 )
		*(long **)arg4 = msg_buf->args[IPC_ARG_INDEX_4];
	switch(msg_buf->type)
	{
		case MESSAGE_TYPE_SHARE_PA:
		case MESSAGE_TYPE_SHARE:
		case MESSAGE_TYPE_VALUE:	/* In value */
			if ( arg5 )
				*(long **)arg5 = msg_buf->args[IPC_ARG_INDEX_5];
			if ( arg6 )
				*(long **)arg6 = msg_buf->args[IPC_ARG_INDEX_6];
			break;
		case MESSAGE_TYPE_REFERENCE:
			/*sanity check*/
			if( IPC_ARGUMENT_LENGTH <= 0 || IPC_ARGUMENT_LENGTH > PAGE_SIZE )
			{
				KTRACE("IPC_ARGUMENT_LENGTH %d\n", IPC_ARGUMENT_LENGTH);
				return ERROR_INVALID_PARAMETER;
			}
			/*copy the entire sender buffer if reciver buffer is big enough else copy only the size of receiver buffer*/
			copy_size = (int)msg_buf->args[IPC_LENGTH_ARG_INDEX];
			if ( copy_size > IPC_ARGUMENT_LENGTH )
				copy_size = IPC_ARGUMENT_LENGTH;
				
			memcpy(IPR_ARGUMENT_ADDRESS, msg_buf->args[IPC_ADDRESS_ARG_INDEX], copy_size);
			kfree(msg_buf->args[IPC_ADDRESS_ARG_INDEX]);
			break;
	}
	
done:
	current_thread = GetCurrentThread();
	current_thread->ipc_reply_to_thread = msg_buf->sender_thread;
	return ERROR_SUCCESS;
}

/*! Waits for the given message queue for message buffer count to be incremented or decremented
 *	\param message_queue 	Message queue for which message buffer needs monitoring
 *	\param wait_time		How long monitoring can happen
 *	\param type				Type of the new buffer will be updated in this variabled
 */
static ERROR_CODE WaitOnMessageQueue(MESSAGE_QUEUE_PTR message_queue, int wait_time, MESSAGE_TYPE * type)
{
	ERROR_CODE ret = ERROR_SUCCESS;
	WAIT_EVENT_PTR	my_wait_event;
	UINT32 ret_time;
	
	/* Wait for this message queue */
	SpinLock( &message_queue->wait_event_msg_queue_lock );
	my_wait_event = AddToEventQueue( &message_queue->wait_event_msg_queue );
	SpinUnlock( &message_queue->wait_event_msg_queue_lock );

	if(wait_time == MESSAGE_QUEUE_NO_WAIT)
		wait_time = 0;

	ret_time = WaitForEvent(my_wait_event, wait_time);
	
	if(ret_time == 0) /* no event fired and we timeout out */
		ret = ERROR_TIMEOUT;
	else
	{
		assert( my_wait_event->fired == 1);
			/*if the caller supplied type variable, fill it*/
		if ( type )
		{
			MESSAGE_BUFFER_PTR buf;
			/* take the lock again because we have returned from blocked state */
			SpinLock( &message_queue->wait_event_msg_queue_lock  ); 
			buf = message_queue->msg_queue;
			if( buf )
				*type = buf->type;
			else
				ret = ERROR_NOT_FOUND;
			
			SpinUnlock( &message_queue->wait_event_msg_queue_lock );
		}
	}

	kfree(my_wait_event);
	
	return ret;
}

/*! Adds the given message buffer to the message queue
 *	\param message_queue	Message queue to which the new message buffer has to inserted
 *	\param buf				Message buffer to be added to the queue
 */
static void AddToMessageQueue(MESSAGE_QUEUE_PTR message_queue, MESSAGE_BUFFER_PTR buf)
{
	assert(message_queue != NULL);
	assert(buf != NULL);
	
	InitList( &buf->message_buffer_queue );
	if(message_queue->msg_queue == NULL)
		message_queue->msg_queue = buf;
	else
		AddToListTail( &message_queue->msg_queue->message_buffer_queue, &buf->message_buffer_queue );
	
	message_queue->buf_count++;
	WakeUpEvent( &message_queue->wait_event_msg_queue, 0);
}

/*! Removes the first message from the given message queue
 *	\param message_queue - message queue to be processed
 */
static void RemoveFromMessageQueue(MESSAGE_QUEUE_PTR message_queue)
{
	MESSAGE_BUFFER_PTR buf;
	
	assert(message_queue != NULL);
	
	buf = message_queue->msg_queue;
	message_queue->buf_count--;
	assert(message_queue->buf_count >= 0);

	if(message_queue->buf_count == 0)
		message_queue->msg_queue = NULL;
	else
		message_queue->msg_queue = STRUCT_ADDRESS_FROM_MEMBER( buf->message_buffer_queue.next, MESSAGE_BUFFER, message_buffer_queue);
	
	RemoveFromList( &buf->message_buffer_queue );
	kfree( buf );
}
