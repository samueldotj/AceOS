/*!
 * \file	include/kernel/ipc.h
 * \brief	Contains declarations for IPC methodologies used.
 */

#ifndef IPC_H
#define IPC_H

typedef struct message_queue MESSAGE_QUEUE, *MESSAGE_QUEUE_PTR;

#include <ace.h>
#include <ds/list.h>
#include <sync/spinlock.h>
#include <kernel/system_call_handler.h>
#include <kernel/wait_event.h>

#define MESSAGE_QUEUE_NO_WAIT -1
#define MESSAGE_QUEUE_CAN_WAIT 0

extern UINT32 max_message_queue_length;	/* System wide tunable to control the size of message queue in task structure */

typedef enum message_type
{
	MESSAGE_TYPE_VALUE,
	MESSAGE_TYPE_REFERENCE,
	MESSAGE_TYPE_SKIP
}MESSAGE_TYPE;

#define ARG_ADDRESS_INDEX	0
#define ARG_LENGTH_INDEX	1
#define ARG1_INDEX			2
#define ARG2_INDEX			3
#define TOTAL_ARG_COUNT		4

typedef struct message_buffer
{
	LIST				message_buffer_queue;
	MESSAGE_TYPE		type;
	UINT32 				args[TOTAL_ARG_COUNT];
}MESSAGE_BUFFER, *MESSAGE_BUFFER_PTR;

struct message_queue
{
	SPIN_LOCK			lock;	/*! Lock to guard the entire mesage queue */
	UINT32				buf_count;	/*! Number of message buffers that are present in this queue */
	MESSAGE_BUFFER_PTR	msg_queue;	/*! Pointer to head of the message buffer queue */
	WAIT_EVENT_PTR		wait_event_msg_queue;	/*! Pointer to wait event queue for message_queue variable */
	SPIN_LOCK			wait_event_msg_queue_lock;	/*! lock for the wait event */
};

ERROR_CODE SendMessage(MESSAGE_TYPE type, register unsigned int arg1, register unsigned int arg2, register unsigned int arg3, register unsigned int arg4, UINT32 pid_target_task, UINT8 queue_no, UINT32 wait_time);
ERROR_CODE SendMessageCore(MESSAGE_TYPE type, register unsigned int arg1, register unsigned int arg2, register unsigned int arg3, register unsigned int arg4, MESSAGE_QUEUE_PTR message_queue, UINT32 wait_time);
ERROR_CODE ReceiveMessage(MESSAGE_TYPE type, unsigned int *arg1, unsigned int *arg2, unsigned int *arg3, unsigned int *arg4, UINT8 queue_no, UINT32 wait_time);
ERROR_CODE ReceiveMessageCore(MESSAGE_TYPE type, unsigned int *arg1, unsigned int *arg2, unsigned int *arg3, unsigned int *arg4, MESSAGE_QUEUE_PTR message_queue, UINT32 wait_time);
UINT32 GetTotalMessageCount(MESSAGE_QUEUE_PTR msg_queue);
void SkipMessage(MESSAGE_QUEUE_PTR msg_queue);
ERROR_CODE GetNextMessageInfo(UINT32 *type, UINT32 *length, MESSAGE_QUEUE_PTR msg_queue);



#endif
