/*!
    \file   kernel/vfs/boot_fs.c
    \brief  File system for loading initram disk
*/

#include <ace.h>
#include <string.h>
#include <tar.h>
#include <ds/lrulist.h>
#include <sync/spinlock.h>
#include <kernel/debug.h>
#include <kernel/ipc.h>
#include <kernel/pm/pm_types.h>
#include <kernel/pm/thread.h>
#include <kernel/pm/scheduler.h>
#include <kernel/vfs/vfs.h>

#define BOOT_FS_NAME			"boot fs"
#define BOOT_FS_MOUNT_DEVICE	"boot_device"
#define BOOT_FS_MOUNT_PATH		"/boot"

#define BOOT_FS_TIME_OUT		5

MESSAGE_QUEUE boot_fs_message_queue;

static void BootFsMessageReceiver();
static void ProcessVfsMessage( MESSAGE_TYPE message_type, VFS_IPC vfs_id, IPC_ARG_TYPE arg2, IPC_ARG_TYPE arg3, IPC_ARG_TYPE arg4, IPC_ARG_TYPE arg5, IPC_ARG_TYPE arg6 );
static FILE_STAT_PARAM_PTR GetDirectoryEntries(void * fs_data, int inode, char * file_name, int max_entries, int * total_entries);
static ERROR_CODE MapTarFile(long inode, long offset, void * va, long size);

/*! contains total directory entries in the boot module tar file*/
static int bootfs_total_directory_entries=0;
/*! virtual address of boot tar*/
static void * bootfs_tar_va=NULL;

/*! Registers the boot file system and mounts /boot mount point
	\note This function runs as boot thread and not in context of bootfs thread
*/
void InitBootFs()
{
	ERROR_CODE ret;
	
	InitMessageQueue( &boot_fs_message_queue );

	/*Create a receiver thread*/
	CreateThread( &kernel_task, BootFsMessageReceiver, SCHED_CLASS_HIGH, TRUE );

	/*register and mount boot file system*/
	ret = RegisterFileSystem( BOOT_FS_NAME, &boot_fs_message_queue );
	if ( ret != ERROR_SUCCESS )
	{
		kprintf("%s\n", ERROR_CODE_AS_STRING(ret) );
		panic( "boot_fs registeration failed" );
	}
		
	ret = MountFileSystem( BOOT_FS_NAME, BOOT_FS_MOUNT_DEVICE, BOOT_FS_MOUNT_PATH );
	if ( ret != ERROR_SUCCESS )
	{
		kprintf("%s\n", ERROR_CODE_AS_STRING(ret) );
		panic( "boot_fs mount failed" );
	}		
		
	bootfs_tar_va = (void *)kernel_reserve_range.module_va_start;
	bootfs_total_directory_entries = GetDirectoryEntryCountInTar(bootfs_tar_va);	
}

static inline ERROR_CODE GetVfsMessage(MESSAGE_TYPE * type, IPC_ARG_TYPE * arg1, IPC_ARG_TYPE * arg2, IPC_ARG_TYPE * arg3, IPC_ARG_TYPE * arg4, IPC_ARG_TYPE * arg5, IPC_ARG_TYPE * arg6)
{
	ERROR_CODE err;
	char * buf;
	UINT32 length = 0;
	while(1)
	{
		err = GetNextMessageInfo( &boot_fs_message_queue, type, &length, 0 );
		if ( err == ERROR_SUCCESS )
			break;
	}
		
	if ( *type == MESSAGE_TYPE_REFERENCE )
	{
		buf = kmalloc(length, 0);
		if ( buf == NULL )
			return ERROR_NOT_ENOUGH_MEMORY;
		err = ReceiveMessageCore( &boot_fs_message_queue, arg1, arg2, arg3, arg4, buf, (IPC_ARG_TYPE)length, BOOT_FS_TIME_OUT );
		if ( err == ERROR_SUCCESS )
		{
			*(char **)ipc_arg_address = buf;
			*(long *)ipc_arg_length = length;
		}
	}
	else if ( *type == MESSAGE_TYPE_VALUE || *type == MESSAGE_TYPE_SHARE )
		err = ReceiveMessageCore( &boot_fs_message_queue, arg1, arg2, arg3, arg4, arg5, arg6, BOOT_FS_TIME_OUT );
	else/*skip the message*/
		err = ReceiveMessageCore( &boot_fs_message_queue, NULL, NULL, NULL, NULL, NULL, NULL, 0 );
		
	return err;
}

/*! Processes VFS message from VFS server*/
static void BootFsMessageReceiver()
{
	ERROR_CODE err;
	MESSAGE_TYPE type;	
	IPC_ARG_TYPE arg1, arg2, arg3, arg4, arg5, arg6;

	while ( 1 )
	{
		err = GetVfsMessage( &type, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6 );
		
		if ( err == ERROR_SUCCESS )
			ProcessVfsMessage( type, (VFS_IPC)arg1, arg2, arg3, arg4, arg5, arg6 );
		else
			kprintf( "bootfs IPC message receive error : %d\n", err );
	}
	kprintf( "Exiting bootfs\n" );
}

static void ProcessVfsMessage( MESSAGE_TYPE message_type, VFS_IPC vfs_id, IPC_ARG_TYPE arg2, IPC_ARG_TYPE arg3, IPC_ARG_TYPE arg4, IPC_ARG_TYPE arg5, IPC_ARG_TYPE arg6 )
{
	FILE_STAT_PARAM_PTR de;
	int total_entries;
	DIRECTORY_ENTRY_PARAM_PTR de_param;
	switch ( vfs_id )
	{
		case VFS_IPC_MOUNT:
			assert( message_type== MESSAGE_TYPE_REFERENCE );
			assert( ipc_arg_address != NULL );
			/*bootfs supports mounting only from boot device*/
			if ( strcmp(ipc_arg_address, BOOT_FS_MOUNT_DEVICE) == 0 )
				ReplyToLastMessage( MESSAGE_TYPE_VALUE, (IPC_ARG_TYPE)VFS_RETURN_CODE_SUCCESS, NULL, NULL, NULL, NULL, NULL );
			else
				ReplyToLastMessage( MESSAGE_TYPE_VALUE, (IPC_ARG_TYPE)VFS_RETURN_CODE_INVALID_PARAMETER, NULL, NULL, NULL, NULL, NULL  );
			break;
		case VFS_IPC_UNMOUNT:
			/*boot mount cant be unmounted*/
			ReplyToLastMessage( MESSAGE_TYPE_VALUE, (IPC_ARG_TYPE)VFS_RETURN_CODE_OPERATION_NOT_SUPPORTED, NULL, NULL, NULL, NULL, NULL  );
			break;
		case VFS_IPC_GET_DIR_ENTRIES:
			assert( message_type == MESSAGE_TYPE_REFERENCE );
			de_param = (DIRECTORY_ENTRY_PARAM_PTR )ipc_arg_address;
			de = GetDirectoryEntries( arg2, -1, NULL, de_param->max_entries, &total_entries);
			if( total_entries > 0 )
				ReplyToLastMessage( MESSAGE_TYPE_REFERENCE, (IPC_ARG_TYPE)VFS_RETURN_CODE_SUCCESS, (IPC_ARG_TYPE)total_entries, NULL, NULL, de, (IPC_ARG_TYPE) (sizeof(FILE_STAT_PARAM)*total_entries));
			else
				ReplyToLastMessage( MESSAGE_TYPE_VALUE, (IPC_ARG_TYPE)VFS_RETURN_CODE_NOT_FOUND, NULL, NULL, NULL, NULL, NULL  );
			break;
		case VFS_IPC_GET_FILE_STAT_PATH:
			assert( message_type == MESSAGE_TYPE_REFERENCE );
			de = GetDirectoryEntries( arg2, -1, ipc_arg_address, 1, NULL );
			if( de )
				ReplyToLastMessage( MESSAGE_TYPE_REFERENCE, (IPC_ARG_TYPE)VFS_RETURN_CODE_SUCCESS, (IPC_ARG_TYPE)1, NULL, NULL, de, (IPC_ARG_TYPE) sizeof(FILE_STAT_PARAM));
			else
				ReplyToLastMessage( MESSAGE_TYPE_VALUE, (IPC_ARG_TYPE)VFS_RETURN_CODE_NOT_FOUND, NULL, NULL, NULL, NULL, NULL  );
			break;
		case VFS_IPC_GET_FILE_STAT_INODE:
			assert( message_type == MESSAGE_TYPE_VALUE );
			de = GetDirectoryEntries( arg2, (int)arg3, NULL, 1, NULL );
			if( de )
				ReplyToLastMessage( MESSAGE_TYPE_REFERENCE, (IPC_ARG_TYPE)VFS_RETURN_CODE_SUCCESS, (IPC_ARG_TYPE)1, NULL, NULL, de, (IPC_ARG_TYPE) sizeof(FILE_STAT_PARAM) );
			else
				ReplyToLastMessage( MESSAGE_TYPE_VALUE, (IPC_ARG_TYPE)VFS_RETURN_CODE_NOT_FOUND, NULL, NULL, NULL, NULL, NULL  );
			break;
		case VFS_IPC_MAP_FILE_PAGE:
			if ( MapTarFile( (long)arg3, (long)arg4, ipc_arg_address, ipc_arg_length ) == ERROR_SUCCESS )
				ReplyToLastMessage( MESSAGE_TYPE_VALUE, (IPC_ARG_TYPE)VFS_RETURN_CODE_SUCCESS, NULL, NULL, NULL, NULL, NULL  );
			else
				ReplyToLastMessage( MESSAGE_TYPE_VALUE, (IPC_ARG_TYPE)VFS_RETURN_CODE_NOT_FOUND, NULL, NULL, NULL, NULL, NULL  );
			break;
		case VFS_IPC_READ_FILE:
			break;
		case VFS_IPC_WRITE_FILE:
		case VFS_IPC_DELETE_FILE:
		case VFS_IPC_MOVE:
		case VFS_IPC_CREATE_SOFT_LINK:
		case VFS_IPC_CREATE_HARD_LINK:
			ReplyToLastMessage( MESSAGE_TYPE_VALUE, (IPC_ARG_TYPE)VFS_RETURN_CODE_INVALID_REQUEST, NULL, NULL, NULL, NULL, NULL  );
			break;
	}
}

/*! Returns the directory entries for a given directory
	\param fs_data - fs provided data for the directory during open file if any
	\param inode - inode of the file else -1
	\param file_name - name of the file else NULL
	\param max_entries - maximum entries required
	\param total_entries - output - total entries in the array
	\return Array of directory entries
*/
static FILE_STAT_PARAM_PTR GetDirectoryEntries(void * fs_data, int inode, char * file_name, int max_entries, int * total_entries)
{
	int total_directory_entries=bootfs_total_directory_entries;
	FILE_STAT_PARAM_PTR result=NULL;
	if ( total_entries )
		* total_entries = 0;
	if ( max_entries < total_directory_entries)
		total_directory_entries = max_entries;
	result = kmalloc( sizeof(FILE_STAT_PARAM)*total_directory_entries, 0 );
	if ( result == NULL )
		return NULL;
	
	if ( !GetDirectoryEntriesInTar(bootfs_tar_va, file_name, inode, result, total_directory_entries) )
		return NULL;
		
	if ( total_entries )
		* total_entries = total_directory_entries;
	
	return result;
}

static ERROR_CODE MapTarFile(long inode, long offset, void * va, long size)
{
	FILE_STAT_PARAM_PTR fsp;
	void * fs_data;

	fsp = GetDirectoryEntries(0, inode, NULL, 1, NULL);
	if( fsp == NULL )
		return ERROR_NOT_FOUND;
	fs_data = fsp->fs_data;
	assert ( fsp->fs_data != NULL );
	
	assert( IS_PAGE_ALIGNED(offset) );
	assert( IS_PAGE_ALIGNED(va) );
	assert( IS_PAGE_ALIGNED(size) );
	
	if( offset+size > PAGE_ALIGN_UP(fsp->file_size) )
	{
		return ERROR_INVALID_PARAMETER;
	}
		
		
	memcpy(va, (void *)((long)fs_data)+offset, size );
	
	return ERROR_SUCCESS;
}
