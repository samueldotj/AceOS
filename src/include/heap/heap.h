/*!
  \file		src/lib/include/heap/heap.h
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created: Fri Mar 21, 2008  08:55PM
  			Last modified: Sat Apr 05, 2008  01:33PM
  \brief	
*/

#ifndef _HEAP_H_
#define _HEAP_H_

#include <ace.h>
#include <heap/slab_allocator.h>

/* Macros go here */
#define MAX_HEAP_BUCKETS 10
#define MAX_HEAP_BUCKET_SIZE 4096

/* Index into bucket for each bucket size */
#define INDEX_8		0
#define INDEX_16	1
#define INDEX_32	2
#define INDEX_64	3
#define INDEX_128	4
#define INDEX_256	5
#define INDEX_512	6
#define INDEX_1024	7
#define INDEX_2048	8
#define INDEX_4096	9


/* Structure definitions go here */

typedef struct heap {
	CACHE cache_bucket[MAX_HEAP_BUCKETS]; /* List of cache entries in the heap */
} HEAP, *HEAP_PTR;

HEAP_PTR my_heap;

/* Function declarations go here */
CACHE_PTR GetCacheFromBucket(UINT32 size);

#endif
