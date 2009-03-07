/*! \file include/kernel/pm/pid.h
    \brief process id related structures
*/

#ifndef _PID_H_
#define _PID_H_

typedef struct pid_info PID_INFO, * PID_INFO_PTR;

#include <ace.h>
#include <ds/list.h>
#include <ds/avl_tree.h>
#include <kernel/pm/task.h>
#include <kernel/wait_event.h>
/*! maximum process number allowed*/
#define MAX_PROCESS_ID			0xFFFF

struct pid_info
{
	int				pid;			/*! Process Id*/
	TASK_PTR		task;			/*! associated task*/
	int 			ppid;			/*! Parent Process Id*/
	
	int				exited;			/*! true if the process is exited/terminated*/
	int 			exit_status;	/*! exit status*/
	WAIT_EVENT_PTR	wait_event;		/*! if somebody want to watch this process dead event, will wait here*/

	AVL_TREE		tree_node;		/*! tree of used pids - to find a pid faster*/

	int				free_count;		/*! total free pids after this pid*/
	LIST			free_list;		/*! links all used pids which has free pids next to them*/
	LIST			inuse_list;		/*! links all used pids regardless whether they have free pids or not*/
};

CACHE pid_cache;				/*! cache for pid info*/

#ifdef __cplusplus
    extern "C" {
#endif

int PidCacheConstructor(void *buffer);
int PidCacheDestructor(void *buffer);

#ifdef __cplusplus
	}
#endif

#endif
