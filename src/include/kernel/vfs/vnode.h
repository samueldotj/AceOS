/*! \file 	kernel/vfs/vnode.h
    \brief 	virtual node realtead functions
*/

#ifndef VNODE_H
#define VNODE_H

#include <kernel/vfs/vfs.h>

/*! Abstraction of FS specific inode structure*/
struct vnode
{
	SPIN_LOCK				lock;
	int						reference_count;				/*! usage count*/
	
	UINT32					file_size;						/*! size of the file*/
	UINT32					mode;							/*! type, protection*/
	SYSTEM_TIME				created_time;					/*! file creation time*/
	SYSTEM_TIME				modified_time;					/*! file last modification time*/
	
	LIST					hash_table_list;				/*! list for hash table - for faster retrieval*/

	UINT32					inode_number;					/*! inode number */
	MOUNTED_FILE_SYSTEM_PTR	mounted_fs;						/*! mounted file system*/
	void * 					fs_data;						/*! file system specific data*/
	DIRECTORY_ENTRY_PTR		directory_entry;				/*! assoicated directory entry, if any*/
	
	VM_UNIT_PTR				unit_head;						/*! head of link list of all the vm units associated with this vnode*/
	
	AVL_TREE_PTR			page_tree_head;					/*! head of tree of all the virtual pages associated with this vnode*/
};

VNODE_PTR AllocateVnode();
void ReleaseVnode(VNODE_PTR vnode);
void ReferenceVnode(VNODE_PTR vnode, DIRECTORY_ENTRY_PTR de);
void AddVnodeToHashTable(VNODE_PTR vnode);
ERROR_CODE GetVnode(DIRECTORY_ENTRY_PTR de, VNODE_PTR * result);
VNODE_PTR GetVnodeFromFile(int file_id);

int VnodeCacheConstructor(void * buffer);
int VnodeCacheDestructor(void * buffer);

#endif 
