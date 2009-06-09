/*! \file include/kernel/pm/task.h
    \brief task related strcutrues and function declarations
*/

#ifndef _TASK_H_
#define _TASK_H_

#include <ace.h>
#include <ds/list.h>
#include <sync/spinlock.h>
#include <kernel/ipc.h>
#include <kernel/wait_event.h>
#include <kernel/mm/vm.h>
#include <kernel/vfs/vfs.h>
#include <kernel/mm/kmem.h>
#include <kernel/pm/pm_types.h>

/*\todo remove these macros and put it as tunable*/
#define TASK_CACHE_FREE_SLABS_THRESHOLD		100 
#define TASK_CACHE_MIN_SLABS				10
#define TASK_CACHE_MAX_SLABS				50

#define MESSAGE_QUEUES_PER_TASK 			5

#define GET_CURRENT_PID()					GetCurrentTask()->pid_info->pid

struct task
{
	SPIN_LOCK	 		lock;												/*! lock for the entire structure */
	int					reference_count;									/*! Number of references to this structure*/
	
	PID_INFO_PTR		pid_info;											/*! Process id information*/
	
	VIRTUAL_MAP_PTR		virtual_map;										/*! virtual map for this task */
	
	THREAD_PTR			thread_head;										/*! threads in the same task */

	MESSAGE_QUEUE		message_queue[MESSAGE_QUEUES_PER_TASK];				/*! Pointer to message queue containing IPC messages */
	
	PROCESS_FILE_INFO	process_file_info;									/*! open file info */
	
	char *				kva_command_line;									/*! kernel virtual address of command line*/
	char *				uva_command_line;									/*! user virtual address of command line - once user va is created kva will be freed*/
	
	char *				kva_environment;									/*! kernel virtual address of process environment*/
	char *				uva_environment;									/*! user virtual address of environment - once user va is created kva will be freed*/
	
	char * 				user_scratch;										/*! temporary memory to copy kernel content to user*/
};

extern CACHE task_cache;
extern TASK kernel_task;

#ifdef __cplusplus
    extern "C" {
#endif

int TaskCacheConstructor(void * buffer);
int TaskCacheDestructor(void * buffer);

void InitKernelTask();
TASK_PTR CreateTask(char * exe_file_path, char * command_line, char * environment);
inline TASK_PTR GetCurrentTask();

TASK_PTR PidToTask(int pid);

#ifdef __cplusplus
	}
#endif

#endif
