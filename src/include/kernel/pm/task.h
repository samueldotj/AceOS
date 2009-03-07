/*! \file include/kernel/pm/task.h
    \brief task related strcutrues and function declarations
*/

#ifndef _TASK_H_
#define _TASK_H_

typedef struct task TASK, * TASK_PTR;
#include <ace.h>
#include <ds/list.h>
#include <sync/spinlock.h>
#include <kernel/mm/vm.h>
#include <kernel/mm/kmem.h>
#include <kernel/pm/thread.h>
#include <kernel/pm/pid.h>
#include <kernel/ipc.h>
#include <kernel/wait_event.h>

/*\todo remove these macros and put it as tunable*/
#define TASK_CACHE_FREE_SLABS_THRESHOLD		100 
#define TASK_CACHE_MIN_SLABS				10
#define TASK_CACHE_MAX_SLABS				50

#define MESSAGE_QUEUES_PER_TASK 			5

#define GET_CURRENT_PID()					GetCurrentTask()->pid_info->pid

struct task
{
	SPIN_LOCK	 		lock;					/*! lock for the entire structure */
	int					reference_count;
	
	PID_INFO_PTR		pid_info;
	
	VIRTUAL_MAP_PTR		virtual_map;			/*! virtual map for this task */
	
	THREAD_PTR			thread_head;			/*! threads in the same task */

	SPIN_LOCK			message_queue_lock[MESSAGE_QUEUES_PER_TASK];
	MESSAGE_BUFFER_PTR	message_queue[MESSAGE_QUEUES_PER_TASK];				/*! Pointer to head of the message buffer queue */
	WAIT_EVENT_PTR		wait_event_message_queue[MESSAGE_QUEUES_PER_TASK];	/*! Pointer to wait event queue for message queue variable */
	SPIN_LOCK			wait_event_lock[MESSAGE_QUEUES_PER_TASK];
	UINT32				message_queue_length;		/*! Number of message buffers that can be present in the queue */
};

extern CACHE task_cache;
extern TASK kernel_task;

#ifdef __cplusplus
    extern "C" {
#endif

int TaskCacheConstructor(void * buffer);
int TaskCacheDestructor(void * buffer);

void InitKernelTask();
TASK_PTR CreateTask(char * exe_file_path);
inline TASK_PTR GetCurrentTask();

TASK_PTR PidToTask(int pid);

#ifdef __cplusplus
	}
#endif

#endif
