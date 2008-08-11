/*!
  \file		include/kernel/pm/thread.h
  \brief	Thread related structures and functions
*/
#ifndef _THREAD_H_
#define _THREAD_H_

#include <ace.h>
#include <ds/list.h>
#include <sync/spinlock.h>
#include <kernel/pm/task.h>

typedef enum 
{
	THREAD_STATE_READY			= 1,		/*thread is ready to run*/
	THREAD_STATE_RUN			= 2,		/*thread is running*/
	THREAD_STATE_TERMINATE		= 3,		/*thread is terminating*/
	THREAD_STATE_SLEEP			= 4,		/*thread is sleeping*/
}THREAD_STATE;


typedef struct thread
{
	SPIN_LOCK	 	lock;					/*lock for the entire structure*/
	int				reference_count;
	
	TASK_PTR		task;					/*back pointer to task*/
	LIST			thread_queue;			/*threads in the same task*/
	
	THREAD_STATE	thread_state;			/*Run State*/
}THREAD, * THREAD_PTR;


#ifdef __cplusplus
    extern "C" {
#endif

THREAD_PTR GetCurrentThread();

#ifdef __cplusplus
	}
#endif

#endif
