/*!
  \file		src/lib/include/heap/heap.h
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created: Fri Mar 21, 2008  08:55PM
  			Last modified: Mon Mar 24, 2008  11:48PM
  \brief	
*/

#ifndef _HEAP_H_
#define _HEAP_H_

#include <ace.h>
#include <heap/slab_allocator.h>

/* Macros go here */
#define MAX_HEAP_BUCKETS 10
#define MAX_HEAP_BUCKET_SIZE 8192

/* Index into bucket for each bucket size */
#define INDEX_8	0
#define INDEX_16	1
#define INDEX_32	2
#define INDEX_128	3
#define INDEX_256	4
#define INDEX_512	5
#define INDEX_1024	6
#define INDEX_2048	7
#define INDEX_4096	8
#define INDEX_8192	9


/* Structure definitions go here */

typedef struct heap {
	CACHE_PTR cache_bucket[MAX_HEAP_BUCKETS]; /* List of cache entries in the heap */
} HEAP, *HEAP_PTR;

HEAP_PTR my_heap;


/* Function declarations go here */
CACHE_PTR GetCacheFromBucket(UINT32 size);

#endif
