/*! \file include/kernel/pm/task.h
    \brief task related strcutrues and function declarations
*/

#ifndef _TASK_H_
#define _TASK_H_

#include <ace.h>
#include <ds/list.h>
#include <sync/spinlock.h>
#include <kernel/mm/vm.h>
#include <kernel/pm/thread.h>

typedef struct task
{
	SPIN_LOCK	 	lock;					/*lock for the entire structure*/
	int				reference_count;
	
	VIRTUAL_MAP_PTR	virtual_map;			/*virtual map for this task*/
	
	THREAD_PTR		thread_head;			/*threads in the same task*/
}TASK, * TASK_PTR;

#ifdef __cplusplus
    extern "C" {
#endif


#ifdef __cplusplus
	}
#endif

#endif
