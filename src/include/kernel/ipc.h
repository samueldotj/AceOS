/*!
 * \file	include/kernel/ipc.h
 * \brief	Contains declarations for IPC methodologies used.
 */

#ifndef IPC_H
#define IPC_H

#include <ace.h>
#include <ds/list.h>
#include <sync/spinlock.h>
#include <kernel/system_call_handler.h>
#include <kernel/wait_event.h>
#include <kernel/pm/pm_types.h>

#define MESSAGE_QUEUE_NO_WAIT -1

extern UINT32 max_message_queue_length;	/* System wide tunable to control the size of message queue in task structure */

typedef enum message_type
{
	MESSAGE_TYPE_VALUE,
	MESSAGE_TYPE_REFERENCE,
	MESSAGE_TYPE_SHARE,
	MESSAGE_TYPE_SHARE_PA
}MESSAGE_TYPE, * MESSAGE_TYPE_PTR;

typedef enum
{
	IPC_ARG_INDEX_1,
	IPC_ARG_INDEX_2,
	IPC_ARG_INDEX_3,
	IPC_ARG_INDEX_4,
	IPC_ARG_INDEX_5,
	IPC_ARG_INDEX_6,
	IPC_ARG_COUNT
}IPC_ARG_INDEX;

/*! index of address argument in the IPC message*/
#define IPC_ADDRESS_ARG_INDEX 	IPC_ARG_INDEX_5
/*! index of length argument in the IPC message*/
#define IPC_LENGTH_ARG_INDEX 	IPC_ARG_INDEX_6

/*! alias for IPC address argument*/
#define IPR_ARGUMENT_ADDRESS	((void *)arg5)
/*! alias for IPC length argument*/
#define IPC_ARGUMENT_LENGTH		((long)arg6)

typedef void * IPC_ARG_TYPE;
typedef IPC_ARG_TYPE * IPC_ARG_TYPE_PTR;

typedef struct message_buffer
{
	LIST				message_buffer_queue;		/*! link list of message buffers in this message queue*/
	MESSAGE_TYPE		type;						/*! type of this message*/
	IPC_ARG_TYPE		args[IPC_ARG_COUNT];		/*! argmuents to this message*/
	
	THREAD_PTR			sender_thread;				/*! thread which initiated this message*/
}MESSAGE_BUFFER, *MESSAGE_BUFFER_PTR;

struct message_queue
{
	SPIN_LOCK			lock;						/*! Lock to guard the entire mesage queue */
	UINT32				buf_count;					/*! Number of message buffers that are present in this queue */
	MESSAGE_BUFFER_PTR	msg_queue;					/*! Pointer to head of the message buffer queue */
	WAIT_EVENT_PTR		wait_event_msg_queue;		/*! Pointer to wait event queue for message_queue variable */
	SPIN_LOCK			wait_event_msg_queue_lock;	/*! lock for the wait event */
};

void InitMessageQueue(MESSAGE_QUEUE_PTR message_queue);
ERROR_CODE SendMessage(UINT32 pid_target_task, UINT8 queue_no, MESSAGE_TYPE type, IPC_ARG_TYPE arg1, IPC_ARG_TYPE arg2, IPC_ARG_TYPE arg3, IPC_ARG_TYPE arg4, IPC_ARG_TYPE arg5, IPC_ARG_TYPE arg6, UINT32 wait_time);
ERROR_CODE SendMessageCore(TASK_PTR target_task, MESSAGE_QUEUE_PTR message_queue, MESSAGE_TYPE type, IPC_ARG_TYPE arg1, IPC_ARG_TYPE arg2, IPC_ARG_TYPE arg3, IPC_ARG_TYPE arg4, IPC_ARG_TYPE arg5, IPC_ARG_TYPE arg6, UINT32 wait_time);

ERROR_CODE ReceiveMessage(UINT8 queue_no, IPC_ARG_TYPE arg1, IPC_ARG_TYPE arg2, IPC_ARG_TYPE arg3, IPC_ARG_TYPE arg4, IPC_ARG_TYPE arg5, IPC_ARG_TYPE arg6, UINT32 wait_time);
ERROR_CODE ReceiveMessageCore(MESSAGE_QUEUE_PTR message_queue, IPC_ARG_TYPE arg1, IPC_ARG_TYPE arg2, IPC_ARG_TYPE arg3, IPC_ARG_TYPE arg4, IPC_ARG_TYPE arg5, IPC_ARG_TYPE arg6, UINT32 wait_time);

ERROR_CODE ReplyToLastMessage(MESSAGE_TYPE type, IPC_ARG_TYPE arg1, IPC_ARG_TYPE arg2, IPC_ARG_TYPE arg3, IPC_ARG_TYPE arg4, IPC_ARG_TYPE arg5, IPC_ARG_TYPE arg6);
ERROR_CODE WaitForReply(MESSAGE_TYPE type, IPC_ARG_TYPE arg1, IPC_ARG_TYPE arg2, IPC_ARG_TYPE arg3, IPC_ARG_TYPE arg4, IPC_ARG_TYPE arg5, IPC_ARG_TYPE arg6, int timeout);
void SkipMessage(MESSAGE_QUEUE_PTR msg_queue);
ERROR_CODE GetNextMessageInfo(MESSAGE_QUEUE_PTR message_queue, MESSAGE_TYPE *type, UINT32 *length, int wait_time);

#endif
