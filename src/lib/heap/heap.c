/*!
  \file		src/lib/heap/heap.c
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created: Sat Mar 22, 2008  06:30PM
  			Last modified: Mon Mar 24, 2008  11:53PM
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
