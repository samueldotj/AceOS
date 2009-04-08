/*! \file 	kernel/vfs/dir_entry.h
    \brief 	directory entry and cache realtead functions
*/

#ifndef DIR_ENTRY_H
#define DIR_ENTRY_H

#include <kernel/vfs/vfs.h>

/*! Directory entry for faster file name access*/
struct directory_entry
{
	char * 					name;							/*! full path of the vnode*/
	
	LIST					lru;							/*! lru list for space management*/
	AVL_TREE				name_tree;						/*! tree for faster search*/
	
	UINT32					inode_number;					/*! inode number */
	MOUNTED_FILE_SYSTEM_PTR	mounted_fs;						/*! mounted file system*/
	void * 					fs_data;						/*! file system specific data*/
	VNODE_PTR				vnode;							/*! associated vnode if any*/
};

LIST_PTR AllocateDirEntryLruNode();
void ReuseDirEntryLru(LIST_PTR node);
void FreeDirEntryLru(LIST_PTR node);

ERROR_CODE GetDirectoryEntry(char * file_path, DIRECTORY_ENTRY_PTR * result);

int DirEntryCacheConstructor(void * buffer);
int DirEntryCacheDestructor(void * buffer);


#endif

