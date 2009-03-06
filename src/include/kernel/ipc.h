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

#define MESSAGE_QUEUE_NO_WAIT -1
#define MESSAGE_QUEUE_CAN_WAIT 0

extern UINT32 max_message_queue_length;	/* System wide tunable to control the size of message queue in task structure */

enum MESSAGE_BUFFER_TYPE
{
	MESSAGE_BUFFER_TYPE_VALUE,
	MESSAGE_BUFFER_TYPE_BUFFER
};


typedef struct message_buffer
{
	LIST						message_buffer_queue;
	enum MESSAGE_BUFFER_TYPE	type;	/* type=0 means In VALUE; type=1 means In BUFFER */
	UINT32 						args[TOTAL_SYSTEM_CALL_ARGS];
	UINT32						from_pid;
}MESSAGE_BUFFER, *MESSAGE_BUFFER_PTR;




#include <kernel/pm/task.h>

ERROR_CODE SendMessage(MESSAGE_BUFFER_PTR mbuf, UINT32 pid_target_task, UINT8 queue_no, UINT32 wait_time);
ERROR_CODE ReceiveMessage(MESSAGE_BUFFER_PTR mbuf, UINT8 queue_no, UINT32 wait_time);

#endif
