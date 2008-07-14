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

#define MAX_HEAP_BUCKETS 		12

/* Structure definitions go here */
typedef struct heap 
{
	CACHE cache_bucket[MAX_HEAP_BUCKETS]; /* List of cache entries in the heap */
} HEAP, *HEAP_PTR;

typedef struct heap_data 
{
	int			bucket_index;
	void * 		buffer[0];
}HEAP_DATA, *HEAP_DATA_PTR;

/* Function declarations go here */
int InitHeap(int page_size, void * (*v_alloc)(int size), 
		int (*v_free)(void * va, int size),
		int (*v_protect)(void * va, int size, int protection)
		);

void * AllocateFromHeap(int size);
int FreeToHeap(void * buffer);

int AddMemoryToHeap(char * start_address, char * end_address );

#endif
