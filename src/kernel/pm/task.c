/*!
  \file		task.c
  \brief	
*/

#include <ace.h>
#include <string.h>
#include <kernel/debug.h>
#include <kernel/mm/kmem.h>
#include <kernel/pm/task.h>
#include <kernel/pm/elf.h>

/*! cache for task allocation*/
CACHE task_cache;

/*! kernel task*/
TASK kernel_task;

/*! Initialize the kernel task data structure with correct values*/
void InitKernelTask()
{
	TaskCacheConstructor(&kernel_task);
	kernel_task.virtual_map = &kernel_map;
	kernel_task.reference_count++;
	
	GetCurrentThread()->task = &kernel_task;
}

/*! Create a task by loading a executable file
	\param exe_file_path - executable file path
	\return on success pointer to the task
			on failure null
*/
TASK_PTR CreateTask(char * exe_file_path)
{
	TASK_PTR task;
	UINT32 err, start_address;
	void (* main_entry)()=NULL;
	
	task = AllocateBuffer( &task_cache, CACHE_ALLOC_SLEEP );
	if ( task == NULL )
		return NULL;
	task->virtual_map = CreateVirtualMap(USER_MAP_START_VA, USER_MAP_START_VA);
	
	LoadBootModule( exe_file_path, &start_address, NULL );
	err = LoadElfImage( (ELF_HEADER_PTR)start_address, task->virtual_map, NULL, (void *)&main_entry );
	if ( err != ERROR_SUCCESS || main_entry == NULL )
	{
		kprintf(" failed (error - %d)\n", err );
		return NULL;
	}
	
	CreateThread( task, main_entry, SCHED_PRI_MID, FALSE );
	
	return task;
}
/*! Returns current task*/
inline TASK_PTR GetCurrentTask()
{
	THREAD_PTR thread = GetCurrentThread();
	assert(thread != NULL);
	return thread->task;
}

/*! Internal function used to initialize the task structure*/
int TaskCacheConstructor(void * buffer)
{
	TASK_PTR task = (TASK_PTR)buffer;
	memset(buffer, 0, sizeof(TASK) );
	
	InitSpinLock( &task->lock );
	task->reference_count = 0;
	
	return 0;
}
/*! Internal function used to clear the task structure*/
int TaskCacheDestructor(void * buffer)
{
	TaskCacheConstructor(buffer);
	return 0;
}
