/*!
    \file   kernel/vfs/dir_entry.c
    \brief  directory entry management
*/

#include <ace.h>
#include <string.h>
#include <ds/lrulist.h>
#include <sync/spinlock.h>
#include <kernel/mm/kmem.h>
#include <kernel/vfs/vfs.h>

/*! cache used by directory entry*/
CACHE dir_entry_cache;

COMPARISION_RESULT compare_dir_entry_name(struct binary_tree * node1, struct binary_tree * node2);
static DIRECTORY_ENTRY_PTR SearchDirectoryEntryTree(char * file_path);

/*! Returns directory entry for a given file name/path
	\param file_path - path for which directory entry is requested
	\param result - directory entry will he updated here
*/
ERROR_CODE GetDirectoryEntry(char * file_path, DIRECTORY_ENTRY_PTR * result)
{
	ERROR_CODE ret;
	DIRECTORY_ENTRY_PTR de;
	LIST_PTR lru_node;

	assert(result);
	*result = NULL;
	/*search whether the directory entry is already cached*/
	de = SearchDirectoryEntryTree( file_path );
	if ( de )
	{
		/*lru management*/
		ReferenceLruNode( &fs_control.dir_entry_lru_list, &de->lru );
	}
	else
	{
		/*allocate a directory entry */
		lru_node = AllocateLruNode(&fs_control.dir_entry_lru_list);
		if ( lru_node == NULL )
			return ERROR_NOT_ENOUGH_MEMORY;
		
		de = STRUCT_ADDRESS_FROM_MEMBER(lru_node, DIRECTORY_ENTRY, lru );

		/*copy the file path*/
		de->name = kmalloc( strlen(file_path), KMEM_NO_FAIL );
		strcpy(de->name, file_path);
		
		/*add it to the tree*/
		InsertNodeIntoAvlTree( &fs_control.dir_entry_root, &de->name_tree, 0, compare_dir_entry_name );
	}
	
	ret = GetVnode(de, &de->vnode);
	
	*result = de;
	return ret;
}


/*! Internal function used to allocate a node for direntry lru list*/
LIST_PTR AllocateDirEntryLruNode()
{
	DIRECTORY_ENTRY_PTR dir_ent;
	dir_ent = AllocateBuffer( &dir_entry_cache, CACHE_ALLOC_SLEEP );
	if ( dir_ent == NULL )
		return NULL;
	
	return &dir_ent->lru;
}

/*! Internal function used to reuse a node for direntry lru list*/
void ReuseDirEntryLru(LIST_PTR node)
{
	assert( node != NULL );
	DIRECTORY_ENTRY_PTR dir_entry = STRUCT_ADDRESS_FROM_MEMBER( node, DIRECTORY_ENTRY, lru ) ;
	RemoveNodeFromAvlTree( &fs_control.dir_entry_root, &dir_entry->name_tree, 0, compare_dir_entry_name);
	kfree( dir_entry->name );
	DirEntryCacheDestructor( dir_entry );
}

/*! Internal function used to free a node from direntry lru list*/
void FreeDirEntryLru(LIST_PTR node)
{
	assert( node != NULL );
	DIRECTORY_ENTRY_PTR dir_entry = STRUCT_ADDRESS_FROM_MEMBER( node, DIRECTORY_ENTRY, lru ) ;
	RemoveNodeFromAvlTree( &fs_control.dir_entry_root, &dir_entry->name_tree, 0, compare_dir_entry_name);
	kfree( dir_entry->name );
	FreeBuffer( dir_entry, &dir_entry_cache);
}

/*! Internal function used to initialize the DirEntry structure*/
int DirEntryCacheConstructor(void * buffer)
{
	DIRECTORY_ENTRY_PTR dir_entry = (DIRECTORY_ENTRY_PTR)buffer;
	memset(buffer, 0, sizeof(DIRECTORY_ENTRY) );
	
	InitList( &dir_entry->lru );
	InitAvlTreeNode( &dir_entry->name_tree, 0 );
	
	return 0;
}
/*! Internal function used to clear the DirEntry structure*/
int DirEntryCacheDestructor(void * buffer)
{
	DirEntryCacheConstructor(buffer);
	return 0;
}
/*! Internal function used to compare to two avl tree nodes(dir entry)*/
COMPARISION_RESULT compare_dir_entry_name(struct binary_tree * node1, struct binary_tree * node2)
{
	char * s1, * s2;
	int result;
	
	assert( node1 != NULL );
	assert( node2 != NULL );
	
	s1 = STRUCT_ADDRESS_FROM_MEMBER(STRUCT_ADDRESS_FROM_MEMBER(node1, AVL_TREE, bintree), DIRECTORY_ENTRY, name_tree)->name;
	s2 = STRUCT_ADDRESS_FROM_MEMBER(STRUCT_ADDRESS_FROM_MEMBER(node2, AVL_TREE, bintree), DIRECTORY_ENTRY, name_tree)->name;
	
	result = strcmp(s1, s2);
	if( result == 0 )
		return EQUAL;
	else if ( result < 0 )
		return LESS_THAN;
	else	
		return GREATER_THAN;
}

/*! Search directory entry tree and returns the corresponding directory entry if present
	\param file_path - file path to search
	\return directory_entry
*/
static DIRECTORY_ENTRY_PTR SearchDirectoryEntryTree(char * file_path)
{
	DIRECTORY_ENTRY search;
	AVL_TREE_PTR result;
	
	search.name = file_path;
	result = SearchAvlTree( fs_control.dir_entry_root, &search.name_tree , compare_dir_entry_name );
	if ( result == NULL )
		return NULL;
	return STRUCT_ADDRESS_FROM_MEMBER(result, DIRECTORY_ENTRY, name_tree);
}
