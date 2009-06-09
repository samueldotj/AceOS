/*!
    \file   kernel/vfs/vfs.c
    \brief  virtual file system implementation
*/

#include <ace.h>
#include <string.h>
#include <ds/bits.h>
#include <ds/lrulist.h>
#include <sync/spinlock.h>
#include <kernel/debug.h>
#include <kernel/vfs/vfs.h>
#include <kernel/mm/kmem.h>
#include <kernel/pm/task.h>


FS_CONTROL fs_control;
FS_PARAM fs_param = 
{
		.vnode.free_slabs_threshold 	= 10,
		.vnode.min_buffers				= 100,
		.vnode.max_buffers				= 500,

		.dir_entry.lru_maximum 				= 200,
		.dir_entry.free_slabs_threshold 	= 10,
		.dir_entry.min_buffers				= 100,
		.dir_entry.max_buffers				= 200
};


void InitBootFs();
static OPEN_FILE_INFO_PTR GetFreeOpenFileInfo(int * index);
static inline ERROR_CODE FileIdToOpenFileInfo(int file_id, PROCESS_FILE_INFO_PTR * pf_info, OPEN_FILE_INFO_PTR * op);

/*! Initialize vfs layer*/
void InitVfs()
{
	int i;
	
	InitSpinLock( &fs_control.lock );
	fs_control.registered_file_systems = NULL;
	fs_control.mounted_file_system_head = NULL;
	
	InitLruList( &fs_control.dir_entry_lru_list, fs_param.dir_entry.lru_maximum, AllocateDirEntryLruNode, ReuseDirEntryLru, FreeDirEntryLru );
	fs_control.dir_entry_root = NULL;
	
	for(i=0;i<VNODE_HASH_TABLE_SIZE;i++)
		InitList( &fs_control.vnode_hash_table[i] );
		
	InitBootFs();
}

/*! Registers a new file system with the kernel
	\param filesystem_name - name of the file system
	\param message_queue - message queue for sending messages to this file system
*/
ERROR_CODE RegisterFileSystem(char * filesystem_name, MESSAGE_QUEUE_PTR	message_queue)
{
	FILE_SYSTEM_PTR fs;
	assert( strlen(filesystem_name) < FILE_SYSTEM_NAME );
	assert( message_queue != NULL );
	
	fs = kmalloc( sizeof(FILE_SYSTEM), KMEM_NO_FAIL );
	strcpy( fs->name, filesystem_name );
	fs->message_queue = message_queue;
	fs->count = 0;
	fs->task = GetCurrentTask();
	InitList( &fs->list );
	
	SpinLock( &fs_control.lock );
	if ( fs_control.registered_file_systems == NULL )
		fs_control.registered_file_systems = fs;
	else
		AddToList( &fs_control.registered_file_systems->list, &fs->list );
	SpinUnlock( &fs_control.lock );
	
	return ERROR_SUCCESS;
}

/*! Returns the file system for a given file system name
	\param name - file system name
*/
FILE_SYSTEM_PTR GetFileSystem(char * name)
{
	FILE_SYSTEM_PTR result = NULL;
	LIST_PTR n;
	
	SpinLock( &fs_control.lock );
	if ( strcmp(fs_control.registered_file_systems->name, name) == 0 )
	{
		result = fs_control.registered_file_systems;
		goto done;
	}
	LIST_FOR_EACH(n, &fs_control.registered_file_systems->list )
	{
		FILE_SYSTEM_PTR fs = STRUCT_ADDRESS_FROM_MEMBER(n, FILE_SYSTEM, list);
		if ( strcmp(fs->name, name) == 0 )
		{
			result = fs;
			goto done;
		}
	}

done:
	SpinUnlock( &fs_control.lock );
	return result;
}

/*! Unregisters a filesystem from kernel
	\param fs_name - file system name to unregister
*/
ERROR_CODE UnregisterFileSystem(char * fs_name)
{
	FILE_SYSTEM_PTR file_system;
	
	file_system = GetFileSystem(fs_name);
	
	if ( file_system == NULL )
		return ERROR_NOT_FOUND;
	
	if ( file_system->count > 0 )
		return ERROR_BUSY;
	
	SpinLock( &fs_control.lock );
	if ( fs_control.registered_file_systems == file_system )
	{
		if ( IsListEmpty( &file_system->list ) )
			fs_control.registered_file_systems = NULL;
		else
			fs_control.registered_file_systems = STRUCT_ADDRESS_FROM_MEMBER(&file_system->list.next, FILE_SYSTEM, list );
	}
	SpinUnlock( &fs_control.lock );
	
	RemoveFromList( &file_system->list );
	kfree(file_system);
	
	return ERROR_SUCCESS;
}

/*! Mounts the file system on a given device
	\param fs_name - file system name
	\param device - on which device to mount
	\param mount_path - where to mount
*/
ERROR_CODE MountFileSystem(char * fs_name, char * device, char * mount_path )
{
	FILE_SYSTEM_PTR fs;
	MOUNTED_FILE_SYSTEM_PTR mount;
	VFS_RETURN_CODE fs_result;
	ERROR_CODE ret;
	LIST_PTR tmp;

	/*check whether the device is already mounted*/
	if ( fs_control.mounted_file_system_head  )
	{
		if( strcmp(device, fs_control.mounted_file_system_head->mount_device) == 0 )
			return ERROR_BUSY;
		LIST_FOR_EACH(tmp, &fs_control.mounted_file_system_head->list )
		{
			MOUNTED_FILE_SYSTEM_PTR mount;
			mount = STRUCT_ADDRESS_FROM_MEMBER(tmp, MOUNTED_FILE_SYSTEM, list);
			if( strcmp(device, mount->mount_device) == 0 )
				return ERROR_BUSY;
		}	
	}
	
	/*get the specified file system*/
	fs = GetFileSystem(fs_name);
	if( fs == NULL )
		return ERROR_NOT_FOUND;
	
	/*Send message to the file system to mount*/
	SendMessageCore( fs->task, fs->message_queue, MESSAGE_TYPE_REFERENCE, VFS_IPC_MOUNT, 0, NULL, NULL, (IPC_ARG_TYPE)device,(IPC_ARG_TYPE)strlen(device), VFS_MOUNT_TIME_OUT);
	/*Wait for FS to complete the mount operation*/
	ret = WaitForReply( MESSAGE_TYPE_VALUE, &fs_result, NULL, NULL, NULL, NULL, NULL, VFS_MOUNT_TIME_OUT );
	if ( ret != ERROR_SUCCESS )
		return ret;
	if ( fs_result != VFS_RETURN_CODE_SUCCESS)
		return ERROR_INVALID_PARAMETER;

	/*\todo - replace kmalloc with cache*/
	mount = kmalloc( sizeof(MOUNTED_FILE_SYSTEM), KMEM_NO_FAIL );
	/*Initialize the structure values*/
	strcpy(mount->mount_name, mount_path);
	strcpy(mount->mount_device, device);
	mount->file_system = fs;
	mount->root_entry = NULL;
	InitList(&mount->list);

	SpinLock( &fs_control.lock );
	/*add to the mount list*/
	if( fs_control.mounted_file_system_head == NULL )
		fs_control.mounted_file_system_head = mount;
	else
		AddToList( &fs_control.mounted_file_system_head->list, &mount->list );
		
	SpinUnlock( &fs_control.lock );
		
	fs->count++;	
	return ERROR_SUCCESS;
}

/*! Returns the mount assoicated with the given path
	\param mount_path - path for which mount should be found
*/
MOUNTED_FILE_SYSTEM_PTR GetMount(char * mount_path)
{
	MOUNTED_FILE_SYSTEM_PTR result = NULL;
	LIST_PTR list;
	char mount_name[MAX_MOUNT_NAME];
	
	assert(mount_path != NULL);
	SpinLock( &fs_control.lock );
	
	/*! If no filesystem mounted return */
	if ( fs_control.mounted_file_system_head == NULL )
		goto done;
	
	/*get mount name only*/
	str_get_token(mount_path, 1, PATH_SEPARATOR, &mount_name[1], sizeof(mount_name)-1);
	mount_name[0]='/';
	if ( strcmp(fs_control.mounted_file_system_head->mount_name,  mount_name) == 0 )
	{
			result = fs_control.mounted_file_system_head;
			goto done;
	}
	LIST_FOR_EACH(list, &fs_control.mounted_file_system_head->list )
	{
		MOUNTED_FILE_SYSTEM_PTR mount = STRUCT_ADDRESS_FROM_MEMBER( list, MOUNTED_FILE_SYSTEM, list );
		if ( strcmp(mount->mount_name, mount_name) == 0 )
		{
			result = mount;
			goto done;
		}
	}
	
done:
	SpinUnlock( &fs_control.lock );
	return result;
}

/*! Unmounts a mounted file system
	\param mount_path - mounted path to be unmounted 
*/
ERROR_CODE UnmountFileSystem(char * mount_path)
{
	MOUNTED_FILE_SYSTEM_PTR mount;
	VFS_RETURN_CODE fs_result;
	ERROR_CODE ret;
	
	assert(mount_path != NULL);
	
	mount = GetMount(mount_path);
	if ( mount == NULL )
		return ERROR_NOT_FOUND;

	/*Send message to the file system to unmount*/
	SendMessageCore( mount->file_system->task, mount->file_system->message_queue, MESSAGE_TYPE_REFERENCE, (IPC_ARG_TYPE) VFS_IPC_UNMOUNT, NULL, NULL, NULL, mount_path,(IPC_ARG_TYPE)strlen(mount_path), VFS_MOUNT_TIME_OUT);
	/*Wait for FS to complete the unmount operation*/
	ret = WaitForReply( MESSAGE_TYPE_VALUE, &fs_result, NULL, NULL, NULL, NULL, NULL, VFS_MOUNT_TIME_OUT );
	if ( ret != ERROR_SUCCESS )
		return ret;
		
	if ( fs_result != VFS_RETURN_CODE_SUCCESS)
		return ERROR_BUSY;

	mount->file_system->count--;
	assert( mount->file_system->count > 0 );
	
	SpinLock( &fs_control.lock );
	/*remove the mount from VFS*/
	if ( fs_control.mounted_file_system_head == mount )
	{
		if ( IsListEmpty( &mount->list ) )
			fs_control.mounted_file_system_head = mount;
		else
			fs_control.mounted_file_system_head = STRUCT_ADDRESS_FROM_MEMBER( &mount->list.next, MOUNTED_FILE_SYSTEM, list );
	}
	
	RemoveFromList( &mount->list );
	SpinUnlock( &fs_control.lock );
	
	kfree(mount);
	
	return ERROR_SUCCESS;
}
/*! Opens a given file for read/write and returns the open file id
	\param file_path - path of the file to be opened
	\param access - access type requested
	\param open_flag - open type - create/truncate etc
	\param file_id - opened file id will be updated here
*/
ERROR_CODE OpenFile(char * file_path, VFS_ACCESS_TYPE access, VFS_OPEN_FLAG open_flag, int * file_id)
{
	OPEN_FILE_INFO_PTR open_file_info;
	DIRECTORY_ENTRY_PTR directory_entry;
	ERROR_CODE ret;
	
	assert( file_path != NULL );
	assert( file_id != NULL );
	
	*file_id = -1;
	
	if ( file_path[0] == 0 )
		return ERROR_NOT_FOUND;
		
	/*try to get it from directory entry cache*/
	ret = GetDirectoryEntry(file_path, &directory_entry);

	if( ret != ERROR_SUCCESS )
		return ret;

	/*get a free file info and fill it*/
	open_file_info = GetFreeOpenFileInfo(file_id);
	if ( open_file_info == NULL )
		return ERROR_RESOURCE_SHORTAGE;	

	open_file_info->vnode = directory_entry->vnode;
	open_file_info->mode = open_flag;
	
	return ERROR_SUCCESS;
}

/*! Returns size of the given file
	\param file_id - file for which size information needed
	\param result - file size is updated here
*/
ERROR_CODE GetFileSize(int file_id, long * result)
{
	PROCESS_FILE_INFO_PTR pf_info;
	OPEN_FILE_INFO_PTR op;
	ERROR_CODE err;
	
	*result = 0;
	err = FileIdToOpenFileInfo(file_id, &pf_info, &op);
	if ( err != ERROR_SUCCESS )
		return err;
		
	assert( op->vnode != NULL );
	*result = op->vnode->file_size;
	
	return ERROR_SUCCESS;
}

/*! Closes the given file
	\param file_id - file id to close
*/
ERROR_CODE CloseFile(int file_id)
{
	PROCESS_FILE_INFO_PTR pf_info;
	OPEN_FILE_INFO_PTR op;
	ERROR_CODE err;
	
	err = FileIdToOpenFileInfo(file_id, &pf_info, &op);
	if ( err != ERROR_SUCCESS )
		return err;
	
	/*!\todo flush the buffers if any*/
	ReleaseVnode(op->vnode);
	
	/*mark the slot as free*/
	ClearBitInBitArray(pf_info->bitmap, file_id);
	memset( op, 0, sizeof(OPEN_FILE_INFO) );
	
	return ERROR_SUCCESS;
}

/*! Reads a directory entry of a directory and fill it in buffer
	\param directory_path - directory to read
	\param buffer - buffer to fill resulting directory entries
	\param max_entries - maximum directory entries (size of the buffer)
	\param total_entries - output - total valid directory entries in the buffer
*/
ERROR_CODE ReadDirectory(char * directory_path, FILE_STAT_PARAM_PTR buffer, int max_entries, int * total_entries)
{
	ERROR_CODE ret;
	assert( directory_path != NULL );
	assert( total_entries );
	
	* total_entries = 0;
	memset(buffer, 0, sizeof(FILE_STAT_PARAM)*max_entries);
	/*if the user requested for root directory listing just read all mounted fs and return*/
	if( strcmp(directory_path, "/") == 0 )
	{
		int i=0;
		LIST_PTR list;
		
		/*after boot this condition is always true - because we will have atleast bootfs and devfs mounted devices*/
		if ( fs_control.mounted_file_system_head != NULL )
		{
			/*copy the head of the list*/
			strcpy(buffer[i].name, fs_control.mounted_file_system_head->mount_name);
			/*copy all nodes in the list*/
			LIST_FOR_EACH(list, &fs_control.mounted_file_system_head->list )
			{
				MOUNTED_FILE_SYSTEM_PTR mount = STRUCT_ADDRESS_FROM_MEMBER( list, MOUNTED_FILE_SYSTEM, list );
				i++;
				strcpy(buffer[i].name, mount->mount_name);
				if(i>=max_entries)
					break;
			}
			* total_entries = i+1;
		}
	}
	else
	{
		MOUNTED_FILE_SYSTEM_PTR mount;
		DIRECTORY_ENTRY_PARAM de_param;
		VFS_RETURN_CODE fs_result;
		DIRECTORY_ENTRY_PTR de=NULL;
		
		mount = GetMount(directory_path);
		if ( mount == NULL )
			return ERROR_NOT_FOUND;
		ret = GetDirectoryEntry(directory_path, &de);
		if ( ret != ERROR_SUCCESS )
			return ret;
		de_param.directory_inode = de->inode_number;
		de_param.after_inode = 0;
		de_param.max_entries = max_entries;
		/*Send message to the file system to unmount*/
		SendMessageCore(mount->file_system->task, mount->file_system->message_queue, MESSAGE_TYPE_REFERENCE, (IPC_ARG_TYPE)VFS_IPC_GET_DIR_ENTRIES, mount->fs_data, NULL, NULL, &de_param,(IPC_ARG_TYPE)sizeof(de_param) , VFS_MOUNT_TIME_OUT);
		/*Wait for FS to complete the unmount operation*/
		ret = WaitForReply( MESSAGE_TYPE_REFERENCE, &fs_result, total_entries, NULL, NULL, buffer, (IPC_ARG_TYPE)  (sizeof(FILE_STAT_PARAM)*max_entries), VFS_MOUNT_TIME_OUT );
		if ( ret != ERROR_SUCCESS )
			return ret;
			
		if ( fs_result != VFS_RETURN_CODE_SUCCESS)
			return ERROR_BUSY;

	}
	return ERROR_SUCCESS;
}

/*! Returns a free open file info from the process
	\param id - the resulting file_id
*/
static OPEN_FILE_INFO_PTR GetFreeOpenFileInfo(int * id)
{
	TASK_PTR current_task;
	PROCESS_FILE_INFO_PTR pf_info;
	UINT32 i;
	
	current_task = GetCurrentTask();
	pf_info = &current_task->process_file_info;
	
	/*find a free open file slot*/
	if ( FindFirstClearBitInBitArray( pf_info->bitmap, MAX_OPEN_FILE, &i ) != 0 )
		return NULL;

	/*mark the slot as used*/
	SetBitInBitArray(pf_info->bitmap, i);
	
	/*update the id which we consumed*/
	if ( id )
		*id = i;
	
	return &pf_info->open_file_info[i];
}

/*! Converts file id to process file info and open file info
	\param file_id - opened file id
	\param pf_info - process file info is returned here
	\param op - open file info is updated here
*/
static inline ERROR_CODE FileIdToOpenFileInfo(int file_id, PROCESS_FILE_INFO_PTR * pf_info, OPEN_FILE_INFO_PTR * op)
{
	TASK_PTR current_task;
	
	assert( pf_info != NULL );
	assert( op != NULL );
	
	if( file_id<0 || file_id > MAX_OPEN_FILE )
		return ERROR_INVALID_PARAMETER;
	
	current_task = GetCurrentTask();
	*pf_info = &current_task->process_file_info;
	
	/*if the file is not opened return error*/
	if (  GetBitFromBitArray((*pf_info)->bitmap, file_id) == 0 )
		return ERROR_INVALID_PARAMETER;
	
	*op = &(*pf_info)->open_file_info[file_id];
	
	return ERROR_SUCCESS;
}

/*! Helper function to receive a VFS message - Used by file systems
 *  It blocks until a message arrives in the message queue and fills the given parameters based on the message
 * \param message_queue - message queue on which to wait to receive a message
 * \param wait_time - timeout value or receive operation.
 * \param type - output param - message type that is received
 * \param arg1 - arg6 - ouput params - message parameters
 * \note It dynamically allocates memory to receive MESSAGE_TYPE_REFERENCE messages, these messages can be freed by calling FreeVfsMessage()
 */
ERROR_CODE GetVfsMessage(MESSAGE_QUEUE_PTR message_queue, UINT32 wait_time, MESSAGE_TYPE_PTR type, IPC_ARG_TYPE_PTR arg1, IPC_ARG_TYPE_PTR arg2, IPC_ARG_TYPE_PTR arg3, IPC_ARG_TYPE_PTR arg4, IPC_ARG_TYPE_PTR arg5, IPC_ARG_TYPE_PTR arg6)
{
	ERROR_CODE err;
	char * buf;
	UINT32 length = 0;
	
	/*may be this loop can be removed?*/
	while(1)
	{
		err = GetNextMessageInfo( message_queue, type, &length, 0 );
		if ( err == ERROR_SUCCESS )
			break;
	}
	/*if the message type is by reference then allocate memory and receive the message*/
	if ( *type == MESSAGE_TYPE_REFERENCE )
	{
		buf = kmalloc(length, 0);
		if ( buf == NULL )
			return ERROR_NOT_ENOUGH_MEMORY;
		err = ReceiveMessageCore( message_queue, arg1, arg2, arg3, arg4, buf, (IPC_ARG_TYPE)length, wait_time );
		if ( err == ERROR_SUCCESS )
		{
			*(char **)IPR_ARGUMENT_ADDRESS = buf;
			*(long *)IPC_ARGUMENT_LENGTH = length;
		}
	}
	else if ( *type == MESSAGE_TYPE_VALUE || *type == MESSAGE_TYPE_SHARE ) 
		err = ReceiveMessageCore( message_queue, arg1, arg2, arg3, arg4, arg5, arg6, wait_time );
	else/*skip the message*/
		err = ReceiveMessageCore( message_queue, NULL, NULL, NULL, NULL, NULL, NULL, wait_time );
		
	return err;
}
/*! Helper functions to free a message previously received using GetVfsMessage - Used by file systems */
void FreeVfsMessage(MESSAGE_TYPE * type, IPC_ARG_TYPE * arg1, IPC_ARG_TYPE * arg2, IPC_ARG_TYPE * arg3, IPC_ARG_TYPE * arg4, IPC_ARG_TYPE * arg5, IPC_ARG_TYPE * arg6)
{
	if ( *type == MESSAGE_TYPE_REFERENCE )
	{
		/*free the memory*/
		kfree( *(char **)IPR_ARGUMENT_ADDRESS );
	}
}
