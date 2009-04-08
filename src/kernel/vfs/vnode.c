/*!
    \file   kernel/vfs/vnode.c
    \brief  vnode management
*/

#include <ace.h>
#include <string.h>
#include <ds/lrulist.h>
#include <sync/spinlock.h>
#include <kernel/vfs/vfs.h>
#include <kernel/pm/task.h>

/*! cache used by directory entry*/
CACHE vnode_cache;

static ERROR_CODE GetFileStat(MOUNTED_FILE_SYSTEM_PTR mount, UINT32 inode, char * file, FILE_STAT_PARAM_PTR fsp);

/*! Internal function used to initialize the Vnode structure*/
int VnodeCacheConstructor(void * buffer)
{
	VNODE_PTR vnode = (VNODE_PTR)buffer;
	memset(buffer, 0, sizeof(VNODE) );
	
	InitList( &vnode->hash_table_list );
	
	return 0;
}
/*! Internal function used to clear the Vnode structure*/
int VnodeCacheDestructor(void * buffer)
{
	VnodeCacheConstructor(buffer);
	return 0;
}

/*! References a vnode so that it will hold
	\param vnode - vnode to be referenced
	\param de - directory entry associated if any
*/
void ReferenceVnode(VNODE_PTR vnode, DIRECTORY_ENTRY_PTR de)
{
	assert( vnode!= NULL );
	vnode->reference_count++;
	if ( de )
		vnode->directory_entry = de;
}
/*! Releases a vnode
	\param vnode - vnode to be released
*/
void ReleaseVnode(VNODE_PTR vnode)
{
	assert( vnode!= NULL );
	
	vnode->reference_count--;
	
	if( vnode->reference_count > 0 )
		return;
		
	ReleaseVnodePages(vnode);
	FreeBuffer( vnode, &vnode_cache );
}
void AddVnodeToHashTable(VNODE_PTR vnode)
{
	
}

/*! Get vnode from the given file id
	\param file_id - opened file number
	\return vnode pointer
*/
VNODE_PTR GetVnodeFromFile(int file_id)
{
	TASK_PTR task;
	OPEN_FILE_INFO_PTR op;
	if( file_id < 0 || file_id > MAX_OPEN_FILE )
		return NULL;
	
	task = GetCurrentTask();
	op = &task->process_file_info.open_file_info[file_id];
	return op->vnode;
}

/*! Returns a vnode for a given directory entry
	\param de - directory entry for which vnode needs to be created/returned
	\param result - resulting vnode will be placed here
*/
ERROR_CODE GetVnode(DIRECTORY_ENTRY_PTR de, VNODE_PTR * result)
{
	VNODE_PTR vnode;
	char * file;
	MOUNTED_FILE_SYSTEM_PTR mount;
	FILE_STAT_PARAM fsp;
	ERROR_CODE ret;
	
	assert( de!=NULL );
	assert( result!=NULL );
	
	*result = NULL;
	vnode = de->vnode;
	if ( vnode == NULL)
	{
		UINT32 inode = -1;
		/*\todo - check if it in hash table?*/

		/*create new vnode*/
		vnode = AllocateBuffer( &vnode_cache, 0 );
		if( de->mounted_fs == NULL )
		{
			/*get the file name without mount name*/
			file = strchr(&de->name[1], PATH_SEPARATOR);
			if( file == NULL )
				file = "/";
			
			/*get the mounted file system*/
			mount = GetMount(de->name);
			if ( mount == NULL)
			{
				FreeBuffer(vnode, &vnode_cache);
				return ERROR_INVALID_PATH;	
			}
		}
		
		/*reterive the file information from fs*/
		ret = GetFileStat(mount, inode, file, &fsp);
		if ( ret != ERROR_SUCCESS )
		{
			FreeBuffer(vnode, &vnode_cache);
			return ret;	
		}
		vnode->created_time = fsp.created_time;
		vnode->file_size = fsp.file_size;
		vnode->fs_data = fsp.fs_data;
		vnode->inode_number = fsp.inode;
		vnode->modified_time = fsp.modified_time;
		vnode->mounted_fs = mount;
		AddVnodeToHashTable( vnode );

	}
	ReferenceVnode(vnode, de);
	de->vnode = vnode;
	
	*result = vnode;
	return ERROR_SUCCESS;
}

/*! Communicates with the FS to file_stat for a given file
	\param mount - mounted file system to which I have to communicate
	\param inode - inode number for which file_stat needs to be returned
	\param file - file path(if we dont have inode number)
	\param fsp - result will be updated here
*/
static ERROR_CODE GetFileStat(MOUNTED_FILE_SYSTEM_PTR mount, UINT32 inode, char * file, FILE_STAT_PARAM_PTR fsp)
{
	VFS_RETURN_CODE fs_result;
	ERROR_CODE ret;
	
	assert( mount != NULL );
	assert( fsp != NULL );
	
	/*Send message to the file system to get the stat for a file*/
	if ( inode >=0 )
		SendMessageCore(mount->file_system->task, mount->file_system->message_queue, MESSAGE_TYPE_REFERENCE, (IPC_ARG_TYPE) VFS_IPC_GET_FILE_STAT_PATH, mount->fs_data, NULL, NULL, file, (IPC_ARG_TYPE)strlen(file)+1, VFS_TIME_OUT);
	else
		SendMessageCore(mount->file_system->task, mount->file_system->message_queue, MESSAGE_TYPE_VALUE, (IPC_ARG_TYPE) VFS_IPC_GET_FILE_STAT_INODE, mount->fs_data, NULL, NULL, (IPC_ARG_TYPE)inode, NULL, VFS_TIME_OUT);
	ret = WaitForReply(MESSAGE_TYPE_REFERENCE, &fs_result, NULL, NULL, NULL, fsp, (IPC_ARG_TYPE) sizeof(FILE_STAT_PARAM), VFS_TIME_OUT );
	if ( ret != ERROR_SUCCESS || fs_result != VFS_RETURN_CODE_SUCCESS )
		return ERROR_INVALID_PATH;
	
	return ERROR_SUCCESS;
}


