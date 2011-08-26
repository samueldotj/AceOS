/*!
    \file   kernel/vfs/ubc.c
    \brief  unified buffer cache for vnode
*/

#include <ace.h>
#include <string.h>
#include <ds/bits.h>
#include <ds/lrulist.h>
#include <sync/spinlock.h>
#include <kernel/debug.h>
#include <kernel/mm/vm.h>
#include <kernel/mm/virtual_page.h>
#include <kernel/vfs/vfs.h>
#include <kernel/mm/kmem.h>
#include <kernel/pm/task.h>

static COMPARISION_RESULT ubc_page_compare(BINARY_TREE_PTR node1, BINARY_TREE_PTR node2);

/*! Returns virtual page corresponds to a vnode at file offset*/
VIRTUAL_PAGE_PTR GetVnodePage(VNODE_PTR vnode, VADDR offset)
{
	VIRTUAL_PAGE_PTR vp = NULL;
	offset = PAGE_ALIGN(offset);
	/*search the tree for page with same offset*/
	if ( vnode->page_tree_head != NULL )
	{
		VIRTUAL_PAGE search_key;
		AVL_TREE_PTR result;
		search_key.ubc_info.offset = offset;
		result = SearchAvlTree( vnode->page_tree_head, &search_key.ubc_info.tree, ubc_page_compare);
		if ( result )
			vp = STRUCT_ADDRESS_FROM_MEMBER(result, VIRTUAL_PAGE, ubc_info.tree);
	}
	/*if we dont have page already, allocate one and do the fs IO to fill the content*/
	if ( vp == NULL )
	{
		vp = AllocateVirtualPages(1, VIRTUAL_PAGE_RANGE_TYPE_NORMAL);
		InitAvlTreeNode( &vp->ubc_info.tree, FALSE );
		vp->ubc = 1;
		vp->ubc_info.vnode = vnode;
		vp->ubc_info.offset = offset;
		vp->ubc_info.loaded = 0;
		vp->ubc_info.modified = 0;
	}
	assert(vp != NULL);
	/*if the page is not yet filled with fs data, do it now*/
	if( !vp->ubc_info.loaded )
	{
		ERROR_CODE ret;
		ret = FillUbcPage(vnode, offset, vp);
		if( ret != ERROR_SUCCESS )
		{
			FreeVirtualPages(vp, 1);
			return NULL;
		}
		InsertNodeIntoAvlTree(&vnode->page_tree_head, &vp->ubc_info.tree, FALSE, ubc_page_compare);
		vp->ubc_info.loaded = 1;
	}
	return vp;
}

/*! Fills a virtual page with content from file by doing a FS IO
	\param vp - virtual page
	\param offset - offset from starting of the file
	\param vp - virtual page where the contents should be filled
*/
ERROR_CODE FillUbcPage(VNODE_PTR vnode, VADDR offset, VIRTUAL_PAGE_PTR vp)
{
	FILE_SYSTEM_PTR fs;
	VFS_RETURN_CODE fs_result;
	ERROR_CODE err;
	assert( vp->ubc_info.vnode != NULL );
	fs = vnode->mounted_fs->file_system;
	SendMessageCore(fs->task, fs->message_queue, MESSAGE_TYPE_SHARE_PA, (IPC_ARG_TYPE)VFS_IPC_MAP_FILE_PAGE, vnode->mounted_fs->fs_data, (IPC_ARG_TYPE)vnode->inode_number, (IPC_ARG_TYPE)offset, (IPC_ARG_TYPE)vp->physical_address, (IPC_ARG_TYPE)PAGE_SIZE, VFS_TIME_OUT);
	err = WaitForReply(MESSAGE_TYPE_VALUE, &fs_result, NULL, NULL, NULL, NULL, NULL, VFS_TIME_OUT);
	if ( err != ERROR_SUCCESS || fs_result != VFS_RETURN_CODE_SUCCESS)
		return ERROR_IO_DEVICE;
	return err;
}

/*! Releases all page related to vnode
	\param vnode - vnode for which pages has to be released
*/
ERROR_CODE ReleaseVnodePages(VNODE_PTR vnode)
{
	assert( vnode->reference_count == 0 );
	
	/*if vnode doesnt hae any page return success*/
	if( vnode->page_tree_head == NULL)
		return ERROR_SUCCESS;

	/*remove all the pages*/
	while( vnode->page_tree_head != NULL )
	{
		AVL_TREE_PTR node = vnode->page_tree_head;
		VIRTUAL_PAGE_PTR vp = STRUCT_ADDRESS_FROM_MEMBER(node, VIRTUAL_PAGE, ubc_info.tree);
		
		RemoveNodeFromAvlTree(  &vnode->page_tree_head, node, 0, ubc_page_compare);
		
		FreeVirtualPages(vp, 1);
	}
	
	return ERROR_SUCCESS;
}

/*! internal function to compare two ubc pages - the comparision key is offset in the file*/
static COMPARISION_RESULT ubc_page_compare(BINARY_TREE_PTR node1, BINARY_TREE_PTR node2)
{
	VIRTUAL_PAGE_PTR vp1, vp2;
	
	assert( node1 != NULL );
	assert( node2 != NULL );
	
	vp1 = STRUCT_ADDRESS_FROM_MEMBER(node1, VIRTUAL_PAGE, ubc_info.tree.bintree);
	vp2 = STRUCT_ADDRESS_FROM_MEMBER(node2, VIRTUAL_PAGE, ubc_info.tree.bintree);
	
	if ( vp1->ubc_info.offset > vp2->ubc_info.offset )
		return GREATER_THAN;
	else if (vp1->ubc_info.offset < vp2->ubc_info.offset )
		return LESS_THAN;
	else 
		return EQUAL;
}

