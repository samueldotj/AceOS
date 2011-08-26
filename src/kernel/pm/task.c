/*!
  \file		task.c
  \brief	
*/

#include <ace.h>
#include <string.h>
#include <kernel/debug.h>
#include <kernel/mm/kmem.h>
#include <kernel/pm/pm_types.h>
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
	\param image_type - executable image type (elf, bin, script etc)
	\param creation_flag - flag to alter the functionality of createtask
	\param entry_point - if provided the main starting code address of the text segment is updated here
	\param command_line - command line string to pass to the program
	\param environment - environment variables to pass to the program
	\return on success pointer to the task
			on failure null
*/
TASK_PTR CreateTask(char * exe_file_path, IMAGE_TYPE image_type, UINT32 creation_flag, VADDR * entry_point, char * command_line, char * environment)
{
	TASK_PTR task;
	ERROR_CODE err;
	UINT32 start_address;

	int file_id;
	long file_size;
	void *main_entry=NULL;
	
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
	
	/*allocate kernel memory for command line*/
	if ( command_line )
	{
		task->kva_command_line = kmalloc(strlen(command_line)+1, 0);
		strcpy(task->kva_command_line, command_line);
	}
	/*allocate kernel memory for environment*/
	if ( environment )
	{
		task->kva_environment = kmalloc(strlen(environment)+1, 0);
		strcpy(task->kva_environment, environment);
	}
		
	memset( &task->process_file_info, 0, sizeof(task->process_file_info) );

	if( image_type == IMAGE_TYPE_ELF_FILE  || image_type == IMAGE_TYPE_BIN_FILE)
	{
		/*map the executable in kernel address space*/
		err = OpenFile(&kernel_task, exe_file_path, VFS_ACCESS_TYPE_READ, OPEN_EXISTING, &file_id);
		if ( err != ERROR_SUCCESS )
			goto error;
		err = GetFileSize(&kernel_task, file_id, &file_size);
		if ( err != ERROR_SUCCESS )
			goto error;
		file_size = PAGE_ALIGN_UP(file_size);
		err = MapViewOfFile(file_id, &start_address, PROT_READ, 0, file_size, 0, 0);
		if ( err != ERROR_SUCCESS )
			goto error;
		/*load the image into address space*/
		switch( image_type )
		{
			case IMAGE_TYPE_ELF_FILE:
				err = LoadElfImage( (ELF_HEADER_PTR)start_address, task->virtual_map, NULL, (void *)&main_entry );
				break;
			case IMAGE_TYPE_BIN_FILE:
				main_entry = (void *)(PAGE_SIZE*2);
				err = CopyVirtualAddressRange( GetCurrentVirtualMap(), start_address, file_size, task->virtual_map, (VADDR *)&main_entry, file_size, PROT_READ | PROT_WRITE );
				break;
			case IMAGE_TYPE_BIN_PROGRAM:
				/*just to avoid gcc warning - this case wont come because it is filtered in the "if" condition*/
				break;
		}
		/*\todo -unmap the mapped view*/
		CloseFile(&kernel_task, file_id);

	}
	else if ( image_type == IMAGE_TYPE_BIN_PROGRAM )
	{
		main_entry = (void *)(PAGE_SIZE*20);
		err = CopyVirtualAddressRange( GetCurrentVirtualMap(), (VADDR)exe_file_path, PAGE_SIZE, task->virtual_map, (VADDR *)&main_entry, PAGE_SIZE, PROT_READ | PROT_WRITE );
	}
	else
		panic("Invalid image type");
		
	
	if ( err != ERROR_SUCCESS )
		goto error;
	
	/*create main thread if needed*/
	if ( creation_flag != TASK_CREATION_FLAG_NO_THREAD )
	{
		CreateThread( task, main_entry, SCHED_PRI_MID, FALSE, NULL);
	}
	
	/*upate thread entry point*/
	if( entry_point )
		*entry_point = (VADDR) main_entry;
	
	goto done;
	
error:
	kprintf("CreateTask(%s): %s\n",exe_file_path, ERROR_CODE_AS_STRING(err) );
	/*\todo - free memory*/
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
