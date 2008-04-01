/*!
  \file		src/lib/heap/slab_allocator.c
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created:	Fri Mar 21, 2008  11:30PM
  			Last modified:	Fri Mar 21, 2008  11:30PM
  \brief	Contains functions to manage slab allocator.
*/

#include <ace.h>
#include <heap/slab_allocator.h>
#include <heap/heap.h>

static SLAB_ALLOCATOR_METADATA slab_alloactor_metadata;

#define VM_PAGE_SIZE	slab_alloactor_metadata.vm_page_size
#define VM_ALLOC		slab_alloactor_metadata.VirtualAlloc
#define VM_FREE			slab_alloactor_metadata.VirtualFree
#define VM_PROTECT		slab_alloactor_metadata.VirtualProtect


#ifdef SLAB_DEBUG_ENABLED
	#define SLAB_DEBUG_PAD_SIZE	(sizeof(UINT32))
#endif

#ifdef	SLAB_DEBUG_ENABLED
	#define BUFFER_SIZE(size)		((size)+SLAB_DEBUG_PAD_SIZE)
#else
	#define BUFFER_SIZE(size)		(size)
#endif


/* Function declarations */

static VADDR GetFreeBufferFromSlab(SLAB_PTR free_slab);
static int AddSlabToCache (CACHE_PTR cache_entry);



/* Function definitions */

void InitSlabAllocator(UINT32 page_size, void * (*v_alloc)(int size), 
	void * (*v_free)(void * va, int size),
	void * (*v_protect)(void * va, int size, int protection)  )
{
	VM_PAGE_SIZE = page_size;
	VM_ALLOC = v_alloc;
	VM_FREE = v_free;
	VM_PROTECT = v_protect;
}

/*!
	\brief	 Creates an empty cache of specified size.

	\param
		new_cache: A static cache created in data segment.
		size: size of the cache to be created.
		free_slabs_threshold: Threshold to start VM operation.
		min_slabs: Minimum no of slabs to be present always.
		max_slabs: Maximum no of slabs allowed.
		constructor: Function pointer which initializes the newly created slab.
		destructor: Function pointer which reuses a slab.

	\return	
		Success(0) if cache is created.
		Failure(-1) if cache is not created.
*/
int CacheCreate (CACHE_PTR new_cache, UINT32 size,
		int free_slabs_threshold, int min_slabs, int max_slabs,
		int (*constructor)(void *buffer), 
		int (*destructor)(void *buffer))
{
	new_cache->size = size;
	new_cache->constructor = constructor;
	new_cache->destructor = destructor;
	new_cache->completely_free_slab_list = NULL;
	new_cache->partially_free_slab_list = NULL;
	new_cache->min_slabs = min_slabs; 
	new_cache->max_slabs = max_slabs;
	new_cache->free_slabs_count = 0;
	new_cache->in_use_slab_tree_root = NULL;
	new_cache->free_slabs_threshold = free_slabs_threshold;
	return 0;
}

void* malloc (UINT32 size, CACHE_PTR *cache_entry)
{
	SLAB_PTR free_slab;
	UINT32 free_slabs_count;
	VADDR ret_va;

	if (size > MAX_HEAP_BUCKET_SIZE)
	{
		/* Get vm_pages directly from vm subsystem */
		// TBD
		return (void*)(ret_va);
	}

	if (!(*cache_entry))
	{
		*cache_entry = GetCacheFromBucket(size);
	}

	assert(*cache_entry); /* (*cache_entry) has to be valid at this point */

	/* Obtain a lock to this cache_entry */
	SpinLock(&((*cache_entry)->slock));

	/* Follow the hierarchy to fetch memory */

	/* STEP1: 
	 * Try from partially free slab list.	
	 */
	if ( (*cache_entry)->partially_free_slab_list) {
		/* Get one free buffer and adjust the list. */
		ret_va = GetFreeBufferFromSlab((*cache_entry)->partially_free_slab_list); 
		/* This is 100% success because this list exists only if some buffers are free */
		goto FREE_BUFFER_FOUND;
	}

FREE_SLAB_FETCH:
	/* STEP2:
	 * Try from Completely free slab list
	 */
	free_slabs_count = (*cache_entry)->free_slabs_count;
	
	if (free_slabs_count)
	{
		free_slab = (*cache_entry)->completely_free_slab_list;
		/* Now remove this slab from free slab list and 
		 * insert it into in use slab tree and partially free slab list. 
		 */
		RemoveFromList(&((*cache_entry)->completely_free_slab_list->completely_free_list));
		InsertNodeIntoAvlTree((AVL_TREE_PTR*)&((*cache_entry)->in_use_slab_tree_root),
				(&(free_slab->in_use_tree)));
		
		AddToListTail(&((*cache_entry)->partially_free_slab_list->partially_free_list),
				(&(free_slab->partially_free_list)));

		free_slab->used_buffer_count = 1;
		
		/* Now get one buffer from this slab */	
		ret_va = GetFreeBufferFromSlab(free_slab);
		goto FREE_BUFFER_FOUND;
	}

	/* STEP3:
	 * Try to add some slabs by requesting VM.
	 */
	if (!AddSlabToCache(*cache_entry)) /* SUCCESS */
	{
		goto FREE_SLAB_FETCH;
	} 
	else /* Failure, no memory! */
	{
		return NULL;
	}

FREE_BUFFER_FOUND:
	return (void*)ret_va;
}

/*!
	\brief	 Adds slabs to cache by requesting memory from VM.

	\param
		cache_entry: Pointer to my cache entry.

	\return
		0	if successfully fetched from VM 
		-1 	if failure.
*/
static int AddSlabToCache (CACHE_PTR cache_entry)
{
	/* Request vm for vm_pages and add it to a slab.
	 * No of vm_pages is decided based on size of cache.
	 */
	//TBD

	(cache_entry->free_slabs_count)++;
	return 0;
}



/*!
	\brief	Gets a free Buffer from the given slab.

	\param
		free_slab:	Pointer to the slab

	\return	 Virtual address of the free buffer.
*/
static VADDR GetFreeBufferFromSlab(SLAB_PTR free_slab)
{
	VADDR ret_va;
	
	//TBD
	//100% get a free buffer.
	//If all buffers utilized, remove from list!
	return ret_va;
}
