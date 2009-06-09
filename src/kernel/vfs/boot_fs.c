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

/*! User friendly name of the boot fs*/
#define BOOT_FS_NAME			"boot fs"
/*! virtual device name - can be null*/
#define BOOT_FS_MOUNT_DEVICE	"boot_device"
/*! Where to mount boot fs*/
#define BOOT_FS_MOUNT_PATH		"/boot"

#define BOOT_FS_TIME_OUT		5

/*! messages passed to boot fs is queued up here - It will be processed by boot fs thread*/
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

	/*register boot file system*/
	ret = RegisterFileSystem( BOOT_FS_NAME, &boot_fs_message_queue );
	if ( ret != ERROR_SUCCESS )
	{
		kprintf("%s\n", ERROR_CODE_AS_STRING(ret) );
		panic( "boot_fs registeration failed" );
	}
	/*mount boot fs on a virtual device*/
	ret = MountFileSystem( BOOT_FS_NAME, BOOT_FS_MOUNT_DEVICE, BOOT_FS_MOUNT_PATH );
	if ( ret != ERROR_SUCCESS )
	{
		kprintf("%s\n", ERROR_CODE_AS_STRING(ret) );
		panic( "boot_fs mount failed" );
	}
	
	/*initialize the boot fs meta data*/
	bootfs_tar_va = (void *)kernel_reserve_range.module_va_start;
	bootfs_total_directory_entries = GetDirectoryEntryCountInTar(bootfs_tar_va);	
}

/*! Boot fs thread
 * Processes VFS requests from VFS server and fulfills the requests
 */
static void BootFsMessageReceiver()
{
	ERROR_CODE err;
	MESSAGE_TYPE type;	
	IPC_ARG_TYPE arg1, arg2, arg3, arg4, arg5, arg6;

	while ( 1 )
	{
		err = GetVfsMessage( &boot_fs_message_queue, BOOT_FS_TIME_OUT, &type, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6 );
		if ( err == ERROR_SUCCESS )
			ProcessVfsMessage( type, (VFS_IPC)arg1, arg2, arg3, arg4, arg5, arg6 );
		else
			kprintf( "bootfs IPC message receive error : %d\n", err );
		
		/*!\todo - process unregister/shutdown request and exit this thread*/
	}
	kprintf( "Exiting bootfs\n" );
}

/*! Processes a VFS message and take neccessary action(reply to the VFS)
 * \param message_type - message queue message type - value/reference/shared etc
 * \param vfs_id - VFS message type - mount/unmount/read/write etc
 * \param arg2-6 - Arguments to the message
 * */
static void ProcessVfsMessage( MESSAGE_TYPE message_type, VFS_IPC vfs_id, IPC_ARG_TYPE arg2, IPC_ARG_TYPE arg3, IPC_ARG_TYPE arg4, IPC_ARG_TYPE arg5, IPC_ARG_TYPE arg6 )
{
	FILE_STAT_PARAM_PTR de;
	int total_entries;
	DIRECTORY_ENTRY_PARAM_PTR de_param;
	switch ( vfs_id )
	{
		case VFS_IPC_MOUNT:
			assert( message_type== MESSAGE_TYPE_REFERENCE );
			assert( IPR_ARGUMENT_ADDRESS != NULL );
			/*bootfs supports mounting only from boot device*/
			if ( strcmp(IPR_ARGUMENT_ADDRESS, BOOT_FS_MOUNT_DEVICE) == 0 )
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
			de_param = (DIRECTORY_ENTRY_PARAM_PTR )IPR_ARGUMENT_ADDRESS;
			de = GetDirectoryEntries( arg2, -1, NULL, de_param->max_entries, &total_entries);
			if( total_entries > 0 )
				ReplyToLastMessage( MESSAGE_TYPE_REFERENCE, (IPC_ARG_TYPE)VFS_RETURN_CODE_SUCCESS, (IPC_ARG_TYPE)total_entries, NULL, NULL, de, (IPC_ARG_TYPE) (sizeof(FILE_STAT_PARAM)*total_entries));
			else
				ReplyToLastMessage( MESSAGE_TYPE_VALUE, (IPC_ARG_TYPE)VFS_RETURN_CODE_NOT_FOUND, NULL, NULL, NULL, NULL, NULL  );
			break;
		case VFS_IPC_GET_FILE_STAT_PATH:
			assert( message_type == MESSAGE_TYPE_REFERENCE );
			de = GetDirectoryEntries( arg2, -1, IPR_ARGUMENT_ADDRESS, 1, NULL );
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
			if ( MapTarFile( (long)arg3, (long)arg4, IPR_ARGUMENT_ADDRESS, IPC_ARGUMENT_LENGTH ) == ERROR_SUCCESS )
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
	{
		kfree( result );
		return NULL;
	}
				
	if ( total_entries )
		* total_entries = total_directory_entries;
	
	return result;
}

/*! Memory map a tar file
 * \param inode - index in the tar file
 * \param offset - offset in the content
 * \param va - user va to map
 * \param size - size of the mapping
 */
static ERROR_CODE MapTarFile(long inode, long offset, void * va, long size)
{
	FILE_STAT_PARAM_PTR fsp;
	void * fs_data;

	assert( IS_PAGE_ALIGNED(offset) );
	assert( IS_PAGE_ALIGNED(va) );
	assert( IS_PAGE_ALIGNED(size) );

	/*get the directory entry associated with the inode*/
	fsp = GetDirectoryEntries(0, inode, NULL, 1, NULL);
	if( fsp == NULL )
		return ERROR_NOT_FOUND;
	
	/*file data starting address*/
	fs_data = fsp->fs_data;
	assert ( fsp->fs_data != NULL );
	
	if( offset+size > PAGE_ALIGN_UP(fsp->file_size) )
	{
		return ERROR_INVALID_PARAMETER;
	}
	
	/*!copy the data*/
	memcpy(va, (void *)((long)fs_data)+offset, size );
	
	return ERROR_SUCCESS;
}
