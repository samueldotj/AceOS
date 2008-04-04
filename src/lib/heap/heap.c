/*!
  \file		src/lib/heap/heap.c
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created: Sat Mar 22, 2008  06:30PM
  			Last modified: Sat Apr 05, 2008  01:26AM
  \brief	Maintains heap
*/

#include <ace.h>
#include <heap/heap.h>


/*!
	\brief	 Indexes into the heap's bucket and gets a cache of appropriate size.

	\param
		size: Size in bytes of a cache entry.
	\return
		Success: Returns the appropriate cache pointer.
		FAILURE: Returns NULL.
*/
CACHE_PTR GetCacheFromBucket(UINT32 size)
{
	assert(size>0 && size<=MAX_HEAP_BUCKET_SIZE && my_heap!= NULL);

	if (size <= 8)
	{
		return my_heap->cache_bucket[INDEX_8];
	}
	else if (size <= 16)
	{
		return my_heap->cache_bucket[INDEX_16];
	}
	else if (size <= 32)
	{
		return my_heap->cache_bucket[INDEX_32];
	}
	else if (size <= 128)
	{
		return my_heap->cache_bucket[INDEX_128];
	}
	else if (size <= 256)
	{
		return my_heap->cache_bucket[INDEX_256];
	}
	else if (size <= 512)
	{
		return my_heap->cache_bucket[INDEX_512];
	}
	else if (size <= 1024)
	{
		return my_heap->cache_bucket[INDEX_1024];
	}
	else if (size <= 2048)
	{
		return my_heap->cache_bucket[INDEX_2048];
	} 
	else if (size <= 4096)
	{
		return my_heap->cache_bucket[INDEX_4096];
	}
	else if (size <= 8192)
	{
		return my_heap->cache_bucket[INDEX_8192];
	}
	else
	{
		return NULL;
	}
}



/*!
	\brief	Allocated VA from Heap. 

	\param
		size: Minimum size requested for allocation.
		cache_entry: Double Pointer to cache from which memory is wanted.

	\return	 Free VA.
*/
void* malloc(UINT32 size, CACHE_PTR *cache_entry)
{
	SLAB_PTR free_slab;
	UINT32 free_slabs_count;
	VADDR ret_va;

	if (size > MAX_HEAP_BUCKET_SIZE)
	{
		/* Get vm_pages directly from vm subsystem */
		/* TBD */
		return (void*)(ret_va);
	}

	if (!(*cache_entry))
	{
		*cache_entry = GetCacheFromBucket(size);
	}

	assert(*cache_entry); /* (*cache_entry) has to be valid at this point */

	ret_va = (VADDR)GetVAFromCache(*cache_ptr, NULL);
	return (void*)ret_va;
}

