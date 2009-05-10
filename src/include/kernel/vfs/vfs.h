/*! \file 	kernel/vfs/vfs.h
    \brief 	virtual file system datastructures
*/

#ifndef VFS_H
#define VFS_H

#include <ace.h>
#include <ds/avl_tree.h>
#include <ds/list.h>
#include <ds/lrulist.h>
#include <heap/slab_allocator.h>
#include <kernel/time.h>
#include <kernel/error.h>
#include <kernel/ipc.h>
#include <kernel/mm/vm.h>
#include <kernel/mm/vm_types.h>
#include <kernel/vfs/vfs_types.h>
#include <kernel/vfs/vnode.h>
#include <kernel/vfs/dir_entry.h>
#include <kernel/vfs/ubc.h>

/*! Maximum string length for short file system name*/
#define FILE_SYSTEM_NAME		20
/*! Maximum lenght of a mount*/
#define MAX_MOUNT_NAME			20
/*! Maximum open file per process*/
#define	MAX_OPEN_FILE			25

#define PATH_SEPARATOR			'/'
#define DEVICE_NAME_SIZE		10

/*! Maximum characters in a file name*/
#define MAX_FILE_NAME			50

/*! Maximum characters in a path - yea it looks too small*/
#define MAX_FILE_PATH			200

/*! how many elements in the vnode hash table*/
#define VNODE_HASH_TABLE_SIZE	100

#define VFS_MOUNT_TIME_OUT	1000
#define VFS_TIME_OUT		1000

/*! \todo - move these enums to usr visible include directory*/
typedef enum
{		
	/*arg1																				arg2				arg3				arg4				arg5				arg6				*/
	
	VFS_IPC_MOUNT,					/*Ask FS to mount a device.							device path			NULL				NULLL				NULL				NULL				*/
	VFS_IPC_UNMOUNT,				/*Ask FS to unmount a device.						device path			NULL				NULLL				NULL				NULL				*/
	VFS_IPC_GET_FILE_STAT_PATH,		/*Ask FS to return file info for given filename. 	fs_data 			NULL				NULL				file path			sizeof(file_path)	*/
	VFS_IPC_GET_FILE_STAT_INODE,	/*Ask FS to return file info for given vnode.		fs_data 			inode				NULL				NULL				NULL				*/
	VFS_IPC_GET_DIR_ENTRIES,		/*Ask FS to return dire entries for given dir.		fs_data 			NULL				NULL				dir entry param		sizeof(dir entry param)*/
	VFS_IPC_READ_FILE,
	VFS_IPC_WRITE_FILE,
	VFS_IPC_MAP_FILE_PAGE,			/*Ask FS to copy file conteent at given va			fs_data				inode				file offset			virtual address 	length of data to copy*/
	VFS_IPC_DELETE_FILE,
	VFS_IPC_MOVE,
	VFS_IPC_CREATE_SOFT_LINK,
	VFS_IPC_CREATE_HARD_LINK,
}VFS_IPC;

typedef enum
{
	VFS_RETURN_CODE_SUCCESS,
	VFS_RETURN_CODE_NOT_FOUND,
	VFS_RETURN_CODE_INVALID_PARAMETER,
	VFS_RETURN_CODE_INVALID_REQUEST,
	VFS_RETURN_CODE_OPERATION_NOT_SUPPORTED,
	VFS_RETURN_CODE_INVALID_NAME,
	VFS_RETURN_CODE_BUSY,
	VFS_RETURN_CODE_NO_SPACE,
	VFS_RETURN_CODE_IO_FAILURE,
	VFS_RETURN_CODE_PERMISSION
}VFS_RETURN_CODE;

typedef enum
{
	VFS_ACCESS_TYPE_READ=1,		/*Read access*/
	VFS_ACCESS_TYPE_WRITE=2,	/*Write access*/
	VFS_ACCESS_TYPE_EXECUTE=4,	/*Execute access*/
}VFS_ACCESS_TYPE;

typedef enum
{
	CREATE_ALWAYS,		/*Creates a new file, always*/
	CREATE_NEW,			/*Creates a new file, only if it does not already exist*/
	OPEN_EXISTING,		/*Opens a file or device, only if it exists*/
	TRUNCATE_EXISTING	/*Opens a file and truncates it so that its size is zero bytes, only if it exists*/
}VFS_OPEN_FLAG;

/*! Representation of a file system*/
struct file_system
{
	char 					name[FILE_SYSTEM_NAME];			/*! short name of the FS*/
	int						count;							/*! count of mounts by this FS*/
	
	MESSAGE_QUEUE_PTR		message_queue;					/*! message queue for sending messages to this FS*/
	TASK_PTR				task;							/*! fs task*/
	
	LIST					list;							/*! links all file systems*/
};

/*! File systems that are mounted*/
struct mounted_file_system
{
	FILE_SYSTEM_PTR			file_system;					/*! file system which handles this mount*/
	char					mount_name[MAX_MOUNT_NAME];		/*! where the file system is mounted*/
	char					mount_device[DEVICE_NAME_SIZE];	/*! which device is mounted*/
	
	UINT32					read_only:1,					/*! read only mount?*/
							flags;							/*! other flags*/
	UINT32					block_size;						/*! block size*/
	UINT32					max_file_size;					/*! maxiumum file size possible on this mount*/
		
	DIRECTORY_ENTRY_PTR		root_entry;						/*! root directory entry of the mount*/
	
	void * 					fs_data;						/*! file system specific data*/
	
	LIST					list;							/*! links all mounts*/
};

/*!  Opened file info
	1) Multiple open file info can reference same file with different offset
*/
struct open_file_info
{
	UINT32					mode;							/*! file open mode*/
	
	VNODE_PTR				vnode;							/*! associated vnode*/
	UINT32					file_offset;					/*! file pointer position*/
};

/*! Process specific opened file  and other informations
	Just grouping of file information stored in task structure.
*/
struct process_file_info
{
	UINT32					umask;							/*! default permission for new file - set by umask call*/
	VNODE_PTR				working_directory;				/*! current working directory*/
	OPEN_FILE_INFO			open_file_info[MAX_OPEN_FILE];	/*! max files per process*/
	char					bitmap[MAX_OPEN_FILE/8];		/*! bitmap to maintain free open file info*/
};

/*! FS fills this datastructures and returns to VFS during VFS_FILE_STAT ipc*/
struct file_stat_param
{
	char 					name[MAX_FILE_NAME];			/*! file name*/
	UINT32					inode;							/*! inode of the file*/
	
	UINT32					file_size;						/*! size of the file*/
	UINT32					mode;							/*! type, protection*/
	SYSTEM_TIME				created_time;					/*! file creation time*/
	SYSTEM_TIME				modified_time;					/*! file last modification time*/
	void *					fs_data;						/*! this will passed to the FS while accessing the vnode*/
};

/*! directory entry parameter passed to FS*/
struct directory_entry_param
{
	UINT32	directory_inode;		/*! folder for which directory entries needs to be returned*/
	UINT32	after_inode;			/*! inode of last returned directory entry- directory entries after this only will be returned*/
	int		max_entries;			/*! maximum entries required */
};

/*! Control variables used by vfs*/
struct fs_control
{
	/*\todo - may be this lock can be converted to reader-writer lock or split into two spinlocks*/
	SPIN_LOCK 				lock;										/*! lock to protect global file system data structures*/

	FILE_SYSTEM_PTR 		registered_file_systems;					/*! list of registered file systems*/
	MOUNTED_FILE_SYSTEM_PTR	mounted_file_system_head; 					/*! list of mounted file systems*/
		
	LRU_LIST				dir_entry_lru_list;							/*! directory entry lru list*/
	AVL_TREE_PTR			dir_entry_root;								/*! directory entry tree root*/
	
	LIST					vnode_hash_table[VNODE_HASH_TABLE_SIZE];	/*! vnode hash table*/
};

struct fs_param
{
	struct
	{
		int					lru_maximum;						/*! maximum entry in the dir entry lru*/
		int					free_slabs_threshold;				/*! directory entry cache - free slab limit*/
		int					min_buffers;						/*! directory entry cache - minimum buffers*/
		int					max_buffers;						/*! directory entry cache - maximum buffers*/
	}dir_entry;
	
	struct
	{
		int					free_slabs_threshold;				/*! vnode cache - free slab limit*/
		int					min_buffers;						/*! vnode cache - minimum buffers*/
		int					max_buffers;						/*! vnode cache - maximum buffers*/
	}vnode;
};

#ifdef __cplusplus
    extern "C" {
#endif

extern CACHE dir_entry_cache;
extern CACHE vnode_cache;

extern FS_CONTROL fs_control;
extern FS_PARAM fs_param;

void InitVfs();

ERROR_CODE UnregisterFileSystem(char * fs_name);
ERROR_CODE RegisterFileSystem(char * filesystem_name, MESSAGE_QUEUE_PTR	message_queue);

ERROR_CODE MountFileSystem(char * fs_name, char * device, char * mount_path );
ERROR_CODE UnmountFileSystem(char * mount_path);
MOUNTED_FILE_SYSTEM_PTR GetMount(char * mount_path);

ERROR_CODE OpenFile(char * file_path, VFS_ACCESS_TYPE access, VFS_OPEN_FLAG open_flag, int * file_id);
ERROR_CODE GetFileSize(int file_id, long * result);
ERROR_CODE CloseFile(int file_id);

ERROR_CODE ReadDirectory(char * directory_path, FILE_STAT_PARAM_PTR buffer, int max_entries, int * total_entries);
ERROR_CODE OpenFile(char * file_path, VFS_ACCESS_TYPE access, VFS_OPEN_FLAG open_flag, int * file_id);
ERROR_CODE CloseFile(int file_id);

#ifdef __cplusplus
	}
#endif

#endif

