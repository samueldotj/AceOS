/*!
  \file		src/lib/heap/heap.c
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created: Sat Mar 22, 2008  06:30PM
  			Last modified: Wed Apr 09, 2008  01:02AM
  \brief	Maintains heap
*/

#include <ace.h>
#include <ds/align.h>
#include <heap/heap.h>

static HEAP _heap;

const static int BUCKET_SIZE[MAX_HEAP_BUCKETS] = {
		16,
		32,
		48,
		64,
		96,
		128,
		196,
		256,
		512,
		1024,
		2048,
		4096
	};

#define CACHE_FROM_INDEX(index)		( _heap.cache_bucket[index] )
#define BUCKET_INDEX(size)	(\
							( (size) <= BUCKET_SIZE[0] ) 	? 	0 : \
							( (size) <= BUCKET_SIZE[1] )	?	1:	\
							( (size) <= BUCKET_SIZE[2] )	?	2:	\
							( (size) <= BUCKET_SIZE[3] )	?	3:	\
							( (size) <= BUCKET_SIZE[4] )	?	4:	\
							( (size) <= BUCKET_SIZE[5] )	?	5:	\
							( (size) <= BUCKET_SIZE[6] )	?	6:	\
							( (size) <= BUCKET_SIZE[7] )	?	7:	\
							( (size) <= BUCKET_SIZE[8] )	?	8:	\
							( (size) <= BUCKET_SIZE[9] )	?	9:	\
							( (size) <= BUCKET_SIZE[10] )	?	10:	\
							( (size) <= BUCKET_SIZE[11] )	?	11:	12 \
							)

#define MAX_HEAP_BUCKET_SIZE 	(BUCKET_SIZE[MAX_HEAP_BUCKETS-1])
#define BUCKET_SIZE(index)		(BUCKET_SIZE[index])

/*!
	\brief	Initializes the heap
	\param	page_size -  Size of virtual page.
	\param	v_alloc - Function Pointer to virtual alloc.
	\param	v_free - Function pointer to virtual free.
	\param 	v_protect -	Function pointer to virtual protect.
	\return	 0 on sucess and -1 on failure
*/
int InitHeap( int page_size, void * (*v_alloc)(int size), 
		int (*v_free)(void * va, int size),
		int (*v_protect)(void * va, int size, int protection)
		)
{
	int i;
	
	if ( InitSlabAllocator(page_size, v_alloc, v_free, v_protect ) == -1 )
		return -1;
	for(i=0; i<MAX_HEAP_BUCKETS; i++ )
		InitCache(&CACHE_FROM_INDEX(i), BUCKET_SIZE(i), 0, 0, 0, NULL, NULL);
	return 0;
}

/*!	Allocates memory from the heap
	\param 	size - number of bytes required
	\return 	starting address of the memory on success
			Null on failure
*/
void * AllocateFromHeap(int size)
{
	int bucket_size, bucket_index;
	CACHE_PTR cache_ptr;
	HEAP_DATA_PTR heap_data_ptr;
	
	bucket_size = ALIGN_UP(size + sizeof(HEAP_DATA), 2);
	if ( bucket_size > MAX_HEAP_BUCKET_SIZE )
	{
		//for simplicity we dont support size greater than page size.
		return NULL;
	}
	bucket_index = BUCKET_INDEX(bucket_size);
	cache_ptr = &CACHE_FROM_INDEX(bucket_index);
	heap_data_ptr = (HEAP_DATA_PTR) AllocateBuffer(cache_ptr, 0);
	if ( heap_data_ptr == NULL )
		return NULL;
	heap_data_ptr->bucket_index = bucket_index;
	return heap_data_ptr->buffer;
}

/*!	frees the given memory to heap
	\param buffer - address to free
*/
int FreeToHeap(void * free_buffer)
{
	HEAP_DATA_PTR heap_data_ptr;
	heap_data_ptr = STRUCT_ADDRESS_FROM_MEMBER( free_buffer, HEAP_DATA, buffer[0]);
	return FreeBuffer(heap_data_ptr, &CACHE_FROM_INDEX(heap_data_ptr->bucket_index) );
}

/*!  Adds preallocated memory pages to heap
	\param start_address - starting address of the memory page
	\param end_address - end address of the memory
	
	\return 0 on success
*/
int AddMemoryToHeap(char * start_address, char * end_address )
{
	int bucket_index=0;
	char * addr;
	for(addr=start_address; addr<end_address; )
	{
		CACHE_PTR cache_ptr = &CACHE_FROM_INDEX(bucket_index);
		
		/*add one more page to the cache*/
		if ( addr+cache_ptr->slab_size <= end_address )
		{
			/*add the pages to the cache*/
			if ( AddSlabToCache(cache_ptr, (VADDR)addr) != 0 )
				return -1;
			/*increment the address by allocated size*/
			addr += cache_ptr->slab_size;
		}
		
		/*next bucket */
		bucket_index++;
		if ( bucket_index >= MAX_HEAP_BUCKETS )
			bucket_index=0;
	}
	return 0;
}
