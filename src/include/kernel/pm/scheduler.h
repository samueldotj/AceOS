/*! \file include/kernel/pm/scheduler.h
    \brief scheduler related strcutrues and function declarations
*/

#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include <ace.h>

/*! Default quantum for each thread in milliseconds*/
#define SCHEDULER_DEFAULT_QUANTUM	1000

typedef struct ready_queue * READY_QUEUE_PTR;
typedef struct priority_queue * PRIORITY_QUEUE_PTR;

typedef enum
{
	SCHED_CLASS_VERY_HIGH,
	SCHED_CLASS_HIGH,
	SCHED_CLASS_MID,
	SCHED_CLASS_LOW,
	SCHED_CLASS_VERY_LOW,
	
	MAX_SCHEDULER_CLASS_LEVELS /* This should be the last element of this enum */
}SCHEDULER_CLASS_LEVELS;

#include <kernel/pm/task.h>

typedef enum
{
	SCHED_PRI_VERY_HIGH,
	SCHED_PRI_HIGH,
	SCHED_PRI_MID,
	SCHED_PRI_LOW,
	SCHED_PRI_VERY_LOW,

	SCHEDULER_PRIORITY_LEVELS_PER_CLASS /* This should always be the last element in this anon. */
}SCHEDULER_PRIORITY_LEVELS;

#define MAX_SCHEDULER_PRIORITY_LEVELS	( MAX_SCHEDULER_CLASS_LEVELS * SCHEDULER_PRIORITY_LEVELS_PER_CLASS )
#define DEFAULT_SCHEDULER_PRIORITY		( MAX_SCHEDULER_PRIORITY_LEVELS / 2 )

/*! Priority queue structure used by scheduler */
typedef struct priority_queue
{
	SPIN_LOCK			lock;
	THREAD_PTR			thread_head;	/* List of threads in this priority queue */ 
	INT8				bonus;			/* Bonus indicates how active this priority queue is. More active it is, lesser the bonus value. */
	UINT8				time_slice;		/* default quantum in milli seconds assigned to threads in this priority queue */
	UINT8 				priority;		/* Current priority of this queue */
	READY_QUEUE_PTR		ready_queue;	/* Back pointer to it's ready queue. */
}PRIORITY_QUEUE;

typedef struct ready_queue
{
	SPIN_LOCK			lock;	/*! Lock to protect entire ready queue */
	UINT32 				mask;	/*! used for quick calculation on which priority queue has threads ready for running. 1 bit for every queue */
	PRIORITY_QUEUE_PTR 	priority_queue[MAX_SCHEDULER_PRIORITY_LEVELS];
}READY_QUEUE;

INT8 ModifyThreadPriorityInfo(THREAD_PTR thread, SCHEDULER_CLASS_LEVELS sched_class);
SCHEDULER_CLASS_LEVELS GetThreadPriorityInfo(THREAD_PTR my_thread);
void ScheduleThread(THREAD_PTR in_thread);
void InitScheduler();
ERROR_CODE BindThreadToProcessor(THREAD_PTR thread, int cpu_no);

#endif
