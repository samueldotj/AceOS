/*!
  \file		kernel/pm/pm_types.h
  \brief	all the typedefs of task, thread and ipc
*/
#ifndef PM_TYPES_H
#define PM_TYPES_H

typedef struct task TASK, * TASK_PTR;
typedef struct thread THREAD, * THREAD_PTR;
typedef struct thread_container THREAD_CONTAINER, * THREAD_CONTAINER_PTR;
typedef struct pid_info PID_INFO, * PID_INFO_PTR;

typedef struct wait_event WAIT_EVENT, *WAIT_EVENT_PTR;
typedef struct message_queue MESSAGE_QUEUE, *MESSAGE_QUEUE_PTR;

#endif
