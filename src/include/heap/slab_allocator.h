/*!
  \file		heap/slab_allocator.h
  \brief	This file contains structures and macros to maintain slab_allocator
*/


#ifndef _SLAB_ALLOCATOR_H_
#define _SLAB_ALLOCATOR_H_

#include <ace.h>
#include <ds/avl_tree.h>
#include <ds/list.h>
#include <sync/spinlock.h>

/*flags for cache alloc*/
#define CACHE_ALLOC_NO_SLEEP			1

/* define this macro to enable statistics */
#define SLAB_STAT_ENABLED
/* define this macro to debug the slab alloctor */
#define SLAB_DEBUG_ENABLED

#ifdef SLAB_DEBUG_ENABLED
	#define	slab_debug_options 0
#endif

typedef enum 
{
	SLAB_STATE_NEW		=	-1,
	SLAB_STATE_FREE		=	0,
	SLAB_STATE_MIXED	=	1,
	SLAB_STATE_USED		=	2
}SLAB_STATE;

/*! Contains fields to maintain slab allocator internals*/
typedef struct slab_allocator_metadata
{
	UINT32	vm_page_size;									/*! size of a virtual page - get it from the kernel*/
	UINT32	vm_page_shift;									/*! 1 << vm_page_shift = to get the page size - \todo - automatically calculate it*/
	void * 	(*virtual_alloc)(int size);						/*! Function pointer to allocate memory in page granulrity*/
	int 	(*virtual_free)(void * va, int size);
	int 	(*virtual_protect)(void * va, int size, int protection);
} SLAB_ALLOCATOR_METADATA, * SLAB_ALLOCATOR_METADATA_PTR;

/*! Statistics for a cache*/
typedef struct cache_statistics
{
	UINT32		alloc_calls; 		/*! total allocation calls from the user */
	UINT32		alloc_failures; 	/*! Number of times alloc failed to find a free buffer in the slab list */
	UINT32		free_calls; 		/*! total free calls from the user */
	
	UINT32		vm_alloc_calls; 	/*! total calls to vm to get memory pages */
	UINT32		vm_free_calls; 		/*! total calls to vm to free memory pages */
	
	UINT32		max_slabs_used;
	UINT32		average_slab_usage;
}CACHE_STATISTICS, * CACHE_STATISTICS_PTR;

/*! A slab - one or more vm pages - contains atleast 8 buffers*/
typedef struct slab {
	UINT16		used_buffer_count;		/*! Total used buffer in this slab*/

	LIST		partially_free_list; 	/*! Ordered list of slabs which has at least one free buffer  and atleast one in use buffer.*/
	AVL_TREE	in_use_tree;			/*! Tree of slabs which has at least one in use buffer. */

	LIST		completely_free_list;	/*! It is a list of completely free slabs (all the buffers within the slab are free). */
	
	BYTE		buffer_usage_bitmap[0];	/*! If a bit is set, the corresponding buffer is used; else free */
} SLAB, *SLAB_PTR;

/*! A cache - contains used slabs and free slabs*/
typedef struct cache {
	SPIN_LOCK	slock;

	int			buffer_size; 					/*! Size of buffers available from this cache */
	int			(*constructor)(void * data); 	/*! Initializes a given buffer */
	int			(*destructor)(void * data); 	/*! Points to a function which makes the buffer reusable */

	int			min_buffers; 					/*! Minimum no of buffers to be present always */
	int			max_slabs; 	 					/*! Maximum no of slabs allowed */
	int			free_slabs_threshold; 			/*! Threshold to start VM operation */

	int			total_slabs;					/*! Total slabs in the cache (this count includes both free and in use)*/
	UINT32		free_slabs_count; 				/*! Count of free slabs in the completely free slab list */
	int 		free_buffer_count;				/*! Total buffers free in this cache*/
	
	AVL_TREE_PTR in_use_slab_tree_root;			/*! A tree to store in use slabs, which is used while freeing to find a slab for the given address.*/
	
	SLAB_PTR	completely_free_slab_list_head; /*! List of completely free slabs which can be freed to VM or used again */ 
	
	SLAB_PTR 	partially_free_slab_list_head; /*! List of partially free slabs which have some buffers free */

	UINT32		slab_size;						/*! Size of a slab (including meta data in multiple of page size)*/
	UINT32		slab_metadata_size;				/*! Size of a slab's metadata*/
	UINT32		slab_metadata_offset;			/*! Where the metadata starts in a slab*/
	UINT32 		slab_buffer_count;				/*! Buffers per slab*/
	
#ifdef SLAB_STAT_ENABLED
	CACHE_STATISTICS stat;
#endif
} CACHE, *CACHE_PTR;

/*! initializes the Slab Allocator subsystem*/
int InitSlabAllocator(UINT32 page_size, void * (*v_alloc)(int size), int (*v_free)(void * va, int size), int (*v_protect)(void * va, int size, int protection) );

/*! initializes a cache*/
int InitCache(CACHE_PTR new_cache, UINT32 size, int free_slabs_threshold, int min_slabs, int max_slabs, int (*constructor)(void *), int (*destructor)(void *));

/*! allocates memory from the specified cache*/
void* AllocateBuffer(CACHE_PTR cache_ptr, UINT32 flag);

/*! Frees a buffer in it's slab */
int FreeBuffer(void *buffer, CACHE_PTR cache_ptr);

/*! destroys the specified cache*/
void DestroyCache(CACHE_PTR);

/*! returns the cache statistics*/
CACHE_STATISTICS_PTR GetCacheStatistics(CACHE_PTR cache_ptr);

/*! gives a page to cache*/
int AddSlabToCache(CACHE_PTR cache_ptr, VADDR slab_start);

#endif
