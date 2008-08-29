/*!
  \file		heap/heap.h
  \brief	
*/

#ifndef _HEAP_H_
#define _HEAP_H_

#include <ace.h>
#include <heap/slab_allocator.h>

/*! Total heap buckets*/
#define MAX_HEAP_BUCKETS 		12
#define VM_BUCKET				(MAX_HEAP_BUCKETS+1)

/*! Contains fields to maintain heap internals*/
typedef struct heap_metadata
{
	UINT32	vm_page_size;									/*! size of a virtual page - get it from the kernel*/
	UINT32	vm_page_shift;									/*! 1 << vm_page_shift = to get the page size - \todo - automatically calculate it*/
	void * 	(*virtual_alloc)(int size);						/*! Function pointer to allocate memory in page granulrity*/
	int 	(*virtual_free)(void * va, int size);
	int 	(*virtual_protect)(void * va, int size, int protection);
} HEAP_METADATA, * HEAP_METADATA_PTR;

/*! main heap data structure*/
typedef struct heap 
{
	CACHE cache_bucket[MAX_HEAP_BUCKETS]; /*! List of cache entries in the heap */
} HEAP, *HEAP_PTR;

typedef struct heap_data 
{
	int			bucket_index;
	void * 		buffer[0];
}HEAP_DATA, *HEAP_DATA_PTR;

int InitHeap(int page_size, void * (*v_alloc)(int size),  int (*v_free)(void * va, int size),	int (*v_protect)(void * va, int size, int protection) );
void * AllocateFromHeap(int size);
int FreeToHeap(void * buffer);

int AddMemoryToHeap(char * start_address, char * end_address );

#endif
