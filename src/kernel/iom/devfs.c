/*!
    \file   kernel/iom/devfs.c
    \brief  Device File system interface - /device
*/

#include <ace.h>
#include <string.h>
#include <tar.h>
#include <ds/lrulist.h>
#include <ds/avl_tree.h>
#include <sync/spinlock.h>
#include <kernel/debug.h>
#include <kernel/ipc.h>
#include <kernel/iom/iom.h>
#include <kernel/iom/devfs.h>
#include <kernel/pm/pm_types.h>
#include <kernel/pm/thread.h>
#include <kernel/pm/scheduler.h>
#include <kernel/vfs/vfs.h>

/*! User friendly name of the device fs*/
#define DEV_FS_NAME				"device fs"
/*! virtual device name*/
#define DEV_FS_MOUNT_DEVICE		"dev_device"
/*! Where to mount device fs*/
#define DEV_FS_MOUNT_PATH		"/device"

#define DEV_FS_TIME_OUT			5

/*! messages passed to device fs is queued up here - It will be processed by device fs thread*/
MESSAGE_QUEUE device_fs_message_queue;

/*! root of devfs*/
AVL_TREE_PTR	devfs_root=NULL;

/*! total device files*/
static int devfs_total_directory_entries=0;

/*! cache for devfs metadata*/
CACHE	devfs_cache;

#define DEVFS_CACHE_FREE_SLABS_THRESHOLD	10
#define DEVFS_CACHE_MIN_BUFFERS				20
#define DEVFS_CACHE_MAX_SLABS				30

/*! used as argument to avl tree enumerate function of dev node tree*/
typedef struct devfs_direntry_param
{
	FILE_STAT_PARAM_PTR		file_stat;		/*! starting address of file_stat param array*/
	int						current_index;	/*! current index into file_stat param array*/
	int						max_entries;	/*! max entries in the file_stat param*/
	
	int						result;			/*! result of the enum operation*/
}DEVFS_DIRENTRY_PARAM, * DEVFS_DIRENTRY_PARAM_PTR;

static void DevFsMessageReceiver();
static void ProcessVfsMessage( MESSAGE_TYPE message_type, VFS_IPC vfs_id, IPC_ARG_TYPE arg2, IPC_ARG_TYPE arg3, IPC_ARG_TYPE arg4, IPC_ARG_TYPE arg5, IPC_ARG_TYPE arg6 );
static FILE_STAT_PARAM_PTR GetDirectoryEntries(void * fs_data, int inode, char * file_name, int max_entries, int * total_entries);
int enumerate_devfs_tree_callback(AVL_TREE_PTR node, void * arg);

static COMPARISION_RESULT compare_dev_node_name(struct binary_tree * node1, struct binary_tree * node2);
int DevFsCacheConstructor( void *buffer);
int DevFsCacheDestructor( void *buffer);

/*! Registers the device file system and mounts /device mount point
*/
void InitDevFs()
{
	ERROR_CODE ret;
	
	/*initialize cache object of devfs*/
	if( InitCache(&devfs_cache, sizeof(DEVFS_METADATA), DEVFS_CACHE_FREE_SLABS_THRESHOLD, DEVFS_CACHE_MIN_BUFFERS, DEVFS_CACHE_MAX_SLABS, DevFsCacheConstructor, DevFsCacheDestructor) )
		panic("InitDevFs - cache init failed");	
	
	InitMessageQueue( &device_fs_message_queue );

	/*Create a receiver thread*/
	CreateThread( &kernel_task, DevFsMessageReceiver, SCHED_CLASS_HIGH, TRUE );

	/*register device file system*/
	ret = RegisterFileSystem( DEV_FS_NAME, &device_fs_message_queue );
	if ( ret != ERROR_SUCCESS )
	{
		KPRINTF("%s\n", ERROR_CODE_AS_STRING(ret) );
		panic( "devfs registeration failed" );
	}
	/*mount boot fs on a virtual device*/
	ret = MountFileSystem( DEV_FS_NAME, DEV_FS_MOUNT_DEVICE, DEV_FS_MOUNT_PATH );
	if ( ret != ERROR_SUCCESS )
	{
		KPRINTF("%s\n", ERROR_CODE_AS_STRING(ret) );
		panic( "devfs mount failed" );
	}
}

/*! DevFs thread
 * Processes VFS requests from VFS server and fulfills the requests
 */
static void DevFsMessageReceiver()
{
	ERROR_CODE err;
	MESSAGE_TYPE type;	
	IPC_ARG_TYPE arg1, arg2, arg3, arg4, arg5, arg6;

	while ( 1 )
	{
		err = GetVfsMessage(&device_fs_message_queue, DEV_FS_TIME_OUT, &type, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6 );
		if ( err == ERROR_SUCCESS )
			ProcessVfsMessage( type, (VFS_IPC)arg1, arg2, arg3, arg4, arg5, arg6 );
		else
			kprintf( "devfs IPC message receive error : %d\n", err );
		
		/*!\todo - process unregister/shutdown request and exit this thread*/
	}
	kprintf( "Exiting devfs\n" );
}

/*! Processes a VFS message and take neccessary action(reply to the VFS)
 * \param message_type - message queue message type - value/reference/shared etc
 * \param vfs_id - VFS message type - mount/unmount/read/write etc
 * \param arg2-6 - Arguments to the message
 * */
static void ProcessVfsMessage( MESSAGE_TYPE message_type, VFS_IPC vfs_id, IPC_ARG_TYPE arg2, IPC_ARG_TYPE arg3, IPC_ARG_TYPE arg4, IPC_ARG_TYPE arg5, IPC_ARG_TYPE arg6 )
{
	FILE_STAT_PARAM_PTR de;
	int total_entries=0;
	DIRECTORY_ENTRY_PARAM_PTR de_param;
	
	switch ( vfs_id )
	{
		case VFS_IPC_MOUNT:
			assert( message_type== MESSAGE_TYPE_REFERENCE );
			assert( IPR_ARGUMENT_ADDRESS != NULL );
			/*devfs supports mounting only one device*/
			if ( strcmp(IPR_ARGUMENT_ADDRESS, DEV_FS_MOUNT_DEVICE) == 0 )
				ReplyToLastMessage( MESSAGE_TYPE_VALUE, (IPC_ARG_TYPE)VFS_RETURN_CODE_SUCCESS, NULL, NULL, NULL, NULL, NULL );
			else
				ReplyToLastMessage( MESSAGE_TYPE_VALUE, (IPC_ARG_TYPE)VFS_RETURN_CODE_INVALID_PARAMETER, NULL, NULL, NULL, NULL, NULL  );
			break;
		case VFS_IPC_UNMOUNT:
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
		case VFS_IPC_READ_FILE:
		case VFS_IPC_WRITE_FILE:
		case VFS_IPC_MAP_FILE_PAGE:
		case VFS_IPC_DELETE_FILE:
		case VFS_IPC_MOVE:
		case VFS_IPC_CREATE_SOFT_LINK:
		case VFS_IPC_CREATE_HARD_LINK:
			ReplyToLastMessage( MESSAGE_TYPE_VALUE, (IPC_ARG_TYPE)VFS_RETURN_CODE_INVALID_PARAMETER, NULL, NULL, NULL, NULL, NULL  );
			break;
	}
}

/*! Creates a special device file under /device
 * \param filename - file name to create under /device folder
 * \param device - device object associated
 * */
ERROR_CODE CreateDeviceNode(const char * filename, DEVICE_OBJECT_PTR device)
{
	DEVFS_METADATA_PTR dp=NULL;
	
	assert(device != NULL);
	
	if( filename == NULL || strlen(filename) > DEVFS_FILE_NAME_MAX-1 )
		return ERROR_INVALID_PARAMETER;
	
	dp = AllocateBuffer(&devfs_cache, 0);
	if ( dp == NULL )
		return ERROR_NOT_ENOUGH_MEMORY;
	
	strcpy( dp->name, filename );
	if ( InsertNodeIntoAvlTree(&devfs_root, &dp->tree, 0, compare_dev_node_name ) != 0 )
		return ERROR_INVALID_PARAMETER;
		
	dp->device = device;
		
	devfs_total_directory_entries++;
	
	return ERROR_SUCCESS;
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
	int total_directory_entries = devfs_total_directory_entries;
	FILE_STAT_PARAM_PTR result=NULL;
	DEVFS_DIRENTRY_PARAM param={0};
	
	if ( total_entries )
		* total_entries = 0;
	if ( max_entries < total_directory_entries)
		total_directory_entries = max_entries;
	result = kmalloc( sizeof(FILE_STAT_PARAM)*total_directory_entries, 0 );
	if ( result == NULL )
		return NULL;
		
	param.file_stat = result;
	param.max_entries = max_entries;
	EnumerateAvlTree(devfs_root, enumerate_devfs_tree_callback, &param);
	
	/*if no entry is reterived free the memory and return null*/
	if( param.current_index == 0 )
	{
		kfree( result );
		return NULL;
	}
	
	if ( total_entries )
		* total_entries = param.current_index;
	
	return result;
}

/*! Searches the vm descriptor AVL tree for a particular VA range*/
static COMPARISION_RESULT compare_dev_node_name(struct binary_tree * node1, struct binary_tree * node2)
{
	DEVFS_METADATA_PTR d1, d2;
	int result;
	assert( node1 != NULL );
	assert( node2 != NULL );
	
	d1 = STRUCT_ADDRESS_FROM_MEMBER(node1, DEVFS_METADATA, tree.bintree);
	d2 = STRUCT_ADDRESS_FROM_MEMBER(node2, DEVFS_METADATA, tree.bintree);
	
	result = strcmp( d1->name, d2->name );
	if( result == 0 )
		return EQUAL;
	else if ( result > 0 )
		return GREATER_THAN;
	else
		return LESS_THAN;
}

/*! Enumerates devfs tree and fills the FILE_STAT_PARAM for each node*/
int enumerate_devfs_tree_callback(AVL_TREE_PTR node, void * arg)
{
	DEVFS_METADATA_PTR dm;
	DEVFS_DIRENTRY_PARAM_PTR param;
	FILE_STAT_PARAM_PTR fstat_param;
	
	dm = STRUCT_ADDRESS_FROM_MEMBER(node, DEVFS_METADATA, tree);
	param = (DEVFS_DIRENTRY_PARAM_PTR)arg;

	assert( param->current_index < param->max_entries );
	
	fstat_param = &param->file_stat[ param->current_index ];
	param->current_index++;
	
	/*fill the entry*/
	strcpy( fstat_param->name, dm->name );
	fstat_param->inode = 0;
	fstat_param->file_size = 0;
	fstat_param->mode = 0;
	fstat_param->fs_data = dm->device;	
	
	/*if no more free slot available break enumeration*/
	if ( param->current_index == param->max_entries )
		return 1;
	
	/*continue enumeration*/
	return 0;
}


/*! Internal function used to initialize the devfs metadata structure*/
int DevFsCacheConstructor( void *buffer)
{
	DEVFS_METADATA_PTR dp = (DEVFS_METADATA_PTR) buffer;
	
	dp->name[0]=0;
	InitAvlTreeNode( &dp->tree, 0 );
	
	return 0;
}

/*! Internal function used to clear the devfs metadata structure*/
int DevFsCacheDestructor( void *buffer)
{
	DevFsCacheConstructor( buffer );
	return 0;
}

