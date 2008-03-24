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


/* Static functions go here */
static ALLOCATED_SLAB_TREE_PTR CreateNodeAllocatedSlabTree (SLAB_PTR slab_entry);
int AddSlabToCache (CACHE_PTR cache_entry);


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
		int (*constructor)(CACHE_PTR, SLAB_PTR), 
		int (*destructor)(CACHE_PTR, SLAB_PTR))
{
	new_cache->size = size;
	new_cache->constructor = constructor;
	new_cache->destructor = destructor;
	new_cache->free_slab_list = NULL;
	new_cache->min_slabs = min_slabs; 
	new_cache->max_slabs = max_slabs;
	new_cache->free_slabs_count = 0;
	new_cache->allocated_slab_tree = NULL;
	new_cache->free_buffer_list = NULL;
	new_cache->free_slabs_threshold = free_slabs_threshold;
	return 0;
}

void* malloc (UINT32 size, CACHE_PTR *cache_entry)
{
	SLAB_PTR free_slab;
	FREE_BUFFER_LIST_PTR free_buffer;
	UINT32 free_slabs_count;

	if (size > MAX_HEAP_BUCKET_SIZE)
	{
		/* Get vm_pages directly from vm subsystem */
		// TBD
		return (void*)(free_buffer);
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
	 * Try from free buffer list.	
	 */
	if ( (*cache_entry)->free_buffer_list) {
		/* Get one free buffer and adjust the list. */
		free_buffer = (*cache_entry)->free_buffer_list;
		RemoveFromList(&((*cache_entry)->free_buffer_list->list));
		goto FREE_BUFFER_FOUND;
	}

FREE_SLAB_FETCH:
	/* STEP2:
	 * Try from Completely free slab list
	 */
	free_slabs_count = (*cache_entry)->free_slabs_count;
	
	if (free_slabs_count)
	{
		ALLOCATED_SLAB_TREE_PTR new_node;

		free_slab = (*cache_entry)->free_slab_list;
		/* Now remove this slab from free slab list and 
		 * insert it into allocated slab tree. 
		 */
		RemoveFromList(&((*cache_entry)->free_slab_list->list));
		new_node = CreateNodeAllocatedSlabTree(free_slab);
		InsertNodeIntoAvlTree((AVL_TREE_PTR*)&((*cache_entry)->allocated_slab_tree),
				(AVL_TREE_PTR)(&(new_node->tree)));
		new_node->reference_count = 1;
		
		/* Now get one buffer from this slab */	
		free_buffer = free_slab->free_buffer_list;
		RemoveFromList(&(free_slab->free_buffer_list->list));
		/* Now put the free buffers in this slab into free buffer list */
		AddToListTail(&((*cache_entry)->free_buffer_list->list), &(free_slab->free_buffer_list->list));
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
	return free_buffer;
}

static ALLOCATED_SLAB_TREE_PTR CreateNodeAllocatedSlabTree (SLAB_PTR slab_entry)
{
	/* The new node occupies memory in vm_page tha's meant for meta data */
	ALLOCATED_SLAB_TREE_PTR new_node = (ALLOCATED_SLAB_TREE_PTR)(GET_META_DATA(slab_entry));
	new_node->slab = slab_entry;
	new_node->reference_count = 0;
	new_node->count_virtual_pages = slab_entry->count_virtual_pages;
	return new_node;
}


/*!
	\brief	 Adds slabs to cache by requesting memory from VM.

	\param
		cache_entry: Pointer to my cache entry.

	\return
		0	if successfully fetched from VM 
		-1 	if failure.
*/
int AddSlabToCache (CACHE_PTR cache_entry)
{
	/* Request vm for vm_pages and add it to a slab.
	 * No of vm_pages is decided based on size of cache.
	 */
	//TBD

	(cache_entry->free_slabs_count)++;
	return 0;
}
