/*!
  \file		task.c
  \brief	
*/

#include <ace.h>
#include <string.h>
#include <kernel/debug.h>
#include <kernel/mm/kmem.h>
#include <kernel/pm/task.h>
#include <kernel/pm/thread.h>
#include <kernel/pm/pid.h>
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
	kernel_task.pid_info = InitPid();
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
	int file_id;
	long file_size;
	void (* main_entry)()=NULL;
	
	task = AllocateBuffer( &task_cache, CACHE_ALLOC_SLEEP );
	if ( task == NULL )
		return NULL;
	task->virtual_map = CreateVirtualMap(USER_MAP_START_VA, USER_MAP_START_VA);
	if ( task->virtual_map == NULL )
	{
		FreeBuffer( task, &task_cache );
		return NULL;
	}
	task->pid_info = AllocatePidInfo( GetCurrentTask()->pid_info->pid );
	
	memset( &task->process_file_info, 0, sizeof(task->process_file_info) );

	err = OpenFile(exe_file_path, VFS_ACCESS_TYPE_READ, OPEN_EXISTING, &file_id);
	if ( err != ERROR_SUCCESS )
		goto error;
	err = GetFileSize(file_id, &file_size);
	if ( err != ERROR_SUCCESS )
		goto error;
	file_size = PAGE_ALIGN_UP(file_size);
	err = MapViewOfFile(file_id, &start_address, PROT_READ, 0, file_size, 0, 0);
	if ( err != ERROR_SUCCESS )
		goto error;
	err = LoadElfImage( (ELF_HEADER_PTR)start_address, task->virtual_map, NULL, (void *)&main_entry );
	if ( err != ERROR_SUCCESS )
		goto error;
	
	/*\todo -unmap the mapped view*/
	CloseFile(file_id);
	
	CreateThread( task, main_entry, SCHED_PRI_MID, FALSE );
	goto done;
	
error:
	kprintf("CreateTask(%s): %s\n",exe_file_path, ERROR_CODE_AS_STRING(err) );
	return NULL;
	
done:
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
