/*!
  \file		slab_allocator.h
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created:	Fri Mar 21, 2008  09:04PM
  			Last modified: Mon Mar 24, 2008  12:37PM
  \brief	This file contains structures to maintain slab_allocator
*/


#ifndef _SLAB_ALLOCATOR_H_
#define _SLAB_ALLOCATOR_H_

#include <ace.h>
#include <ds/avl_tree.h>
#include <ds/list.h>
#include <kernel/spinlock.h>

/*define this macro to enable stat*/
#define SLAB_STAT_ENABLED
/*define this macro to debug the slab alloctor*/
#define SLAB_DEBUG_ENABLED

struct slab_allocator_metadata
{
	UINT32	vm_page_size;
	void * (*VirtualAlloc)(int size);
	void * (*VirtualFree)(void * va, int size),
	void * (*VirtualProtect)(void * va, int size, int protection);
}SLAB_ALLOCATOR_METADATA, * SLAB_ALLOCATOR_METADATA_PTR;


#ifdef SLAB_DEBUG_ENABLED
	UINT32	slab_debug_options = 0;
#endif
typedef struct slab {
	UINT16		used_buffer_count;
	LIST		in_use_list;/*It is ordered list of slabs which has at least one free buffer and at least one in use buffer. It is used during buffer allocation to find a free buffer in the slab.*/
	AVL_TREE	in_use_tree;/*It is tree of slabs which has at least one in use buffer. It is used during buffer free to find the slab to which the buffer is belongs to.*/
	LIST		free_list;/*It is list of completely free slabs(all the buffers are free). It is used to release the memory pages back to the vm.*/
	UINT32		buffer_usage_bitmap[0];/*if a bit is set the corresponding buffer is used else free*/
} SLAB, *SLAB_PTR;

typedef struct cache {
	SPIN_LOCK	slock;

	int			size;	/* size of buffers available from this cache */
	int			(*constructor)(void * data);/* initializes a given buffer */
	int			(*destructor)(void * data);/* points to a function whichmakes the buffer reusable */

	int			min_slabs;/* Minimum no of slabs to be present always */
	int			max_slabs;/* Maximum no of slabs allowed */

	int			free_slabs_threshold; /* Threshold to start VM operation */
	int			free_slabs_count; /* count of free slabs in the free slab list */

	SLAB_PTR	in_use_slab_tree_root;/* A tree to store in use slabs which is used while freeing to find the slab for the given address*/
	SLAB_PTR	completely_free_slab_list;/* List of completely free slabs which can be freed to VM or used again*/
	
#ifdef SLAB_STAT_ENABLED
	UINT32		alloc_calls;/*total allocation calls from the user*/
	UINT32		free_calls;/*total free calls from the user*/
	UINT32		vm_alloc_calls;/*total calls to vm to get memory pages*/
	UINT32		vm_free_calls;/*total calls to vm to free memory pages*/
	UINT32		free_buffer_end;/*number of times alloc failed to find a free buffer in slab list*/
	
	UINT32		max_slabs_used;
	UINT32		average_slab_usage;
#endif

} CACHE, *CACHE_PTR;

/*initializes the cache subsystem*/
void InitSlabAllocator(UINT32 page_size, void * (*v_alloc)(int size), 
		void * (*v_free)(void * va, int size),
		void * (*v_protect)(void * va, int size, int protection),
       );

/*initializes a cache*/
int CacheCreate(CACHE_PTR new_cache, UINT32 size, int min_slabs,  int max_slabs, int free_slabs_threshold, int (*constructor)(void *), int (*destructor)(void) );

/*allocates memory from the specified cache*/
void * CacheAlloc(CACHE_PTR);
/*frees memory to the specified cache*/
void CacheFree(CACHE_PTR, void *);
/*destroys the specified cache*/
void CacheDestroy(CACHE_PTR);

#endif
