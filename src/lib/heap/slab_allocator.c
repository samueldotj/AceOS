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
#include <string.h>

static SLAB_ALLOCATOR_METADATA slab_alloactor_metadata;

#define VM_PAGE_SIZE	slab_alloactor_metadata.vm_page_size
#define VM_PAGE_SHIFT	slab_alloactor_metadata.vm_page_shift
#define VM_ALLOC		slab_alloactor_metadata.virtual_alloc
#define VM_FREE			slab_alloactor_metadata.virtual_free
#define VM_PROTECT		slab_alloactor_metadata.virtual_protect

#define VM_PAGE_ALIGN(addr)		((UINT32)(addr) & -VM_PAGE_SIZE)
#define VM_PAGE_ALIGN_UP(addr)	((UINT32)((addr) + VM_PAGE_SIZE - 1) & -VM_PAGE_SIZE)

#ifdef SLAB_DEBUG_ENABLED
	#define SLAB_DEBUG_PAD_SIZE	(sizeof(UINT32))
#endif

#ifdef	SLAB_DEBUG_ENABLED
	#define BUFFER_SIZE(size)		((size)+SLAB_DEBUG_PAD_SIZE)
#else
	#define BUFFER_SIZE(size)		(size)
#endif

/*max size of the slab(buffers+metadata)
	1) slab should contain atleast 8 buffer
	2) slab size includes its meta data
		a) size of the the slab structure
		b) size of the bitmap at the end of the slab strucutre
*/
#define SLAB_SIZE(buffer_size)		(VM_PAGE_ALIGN_UP( ((buffer_size) << 3)+sizeof(SLAB)+1 ) )
/*max number of pages in the slab*/
#define SLAB_PAGES(buffer_size)		( (SLAB_SIZE(buffer_size)) >> VM_PAGE_SHIFT )
/*get the start of slab from slab metadata addresss*/
#define SLAB_START(slab_metadata_ptr, slab_metaoffset)	( ((UINT32)slab_metadata_ptr) - slab_metaoffset )

static COMPARISION_RESULT slab_inuse_tree_compare(AVL_TREE_PTR a, AVL_TREE_PTR b)
{
	return 0;
}

/* Calculates and returns the information about the slab metadata for a given buffer size
		slab_total_size and metadata_size pointers will updated with slab total size and metadata size appropriately.
		returns max number of buffers in the slab
*/
static int GetSlabMetadataInfo(UINT32 buffer_size, UINT32 * slab_total_size, UINT32 * metadata_size, UINT32 * metadata_offset)
{
	int bitmap_size, buf_count, slab_tot_size, meta_size;
	
	slab_tot_size = SLAB_SIZE(buffer_size);
	/*total buffer count*/
	buf_count = (slab_tot_size-sizeof(SLAB)) / buffer_size;
	/*size of the bitmap in bytes*/
	bitmap_size = buf_count / BITS_PER_BYTE;
	/*recalcualte the buffer count*/
	buf_count = (slab_tot_size-sizeof(SLAB)-bitmap_size) / buffer_size;
	
	/*caclulate the metadata size*/
	meta_size = sizeof(SLAB) + buf_count/BITS_PER_BYTE;
	
	if ( slab_total_size )
		* slab_total_size = slab_tot_size;
	if ( metadata_size )
		* metadata_size = meta_size;
	if ( metadata_offset )
		* metadata_offset  = slab_tot_size - meta_size;
	
	return buf_count;
}

static void SlabInit(SLAB_PTR s, UINT32 buffer_count)
{
	/*initialize the tree and list */
	InitList( &s->partially_free_list );
	InitAvlTreeNode( &s->in_use_tree, slab_inuse_tree_compare);
	InitList( &s->completely_free_list );
	
	/*all buffers are free*/
	s->used_buffer_count = 0;
	memset( s->buffer_usage_bitmap, 0, buffer_count/BITS_PER_BYTE);
}

static VADDR GetFreeBufferFromSlab(CACHE_PTR c, SLAB_PTR s);
static int AddSlabToCache(CACHE_PTR c, int add_to_free_list);

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
	\brief	 Initializes an empty cache of specified buffer size.

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
int CacheInit (CACHE_PTR c, UINT32 size,
		int free_slabs_threshold, int min_buffers, int max_slabs,
		int (*constructor)(void *buffer), 
		int (*destructor)(void *buffer))
{
	InitSpinLock( &c->slock);

	c->buffer_size = size;
	c->constructor = constructor;
	c->destructor = destructor;
	
	c->min_buffers = min_buffers;
	c->max_slabs = max_slabs;
	c->free_slabs_threshold = free_slabs_threshold;
	
	c->partially_free_slab_list_head = NULL;
	c->in_use_slab_tree_root = NULL;
	
	c->completely_free_slab_list_head = NULL;
	c->free_slabs_count = 0;
	
	c->free_buffer_count = 0;
	
	c->slab_buffer_count = GetSlabMetadataInfo(size, &c->slab_size, &c->slab_metadata_size, &c->slab_metadata_offset);
	
	return 0;
}
#define ADD_TO_INUSE_TREE(cache_ptr, slab_ptr)	\
	InsertNodeIntoAvlTree( &(cache_ptr)->in_use_slab_tree_root, &(slab_ptr)->in_use_tree );
	
#define REMOVE_FROM_INUSE_TREE(cache_ptr, slab_ptr)	\
	RemoveNodeFromAvlTree( &(cache_ptr)->in_use_slab_tree_root, &(slab_ptr)->in_use_tree );
	
#define ADD_TO_PARTIAL_LIST(cache_ptr, slab_ptr)	\
	AddToList( &(cache_ptr)->partially_free_slab_list_head->partially_free_list , &(slab_ptr)->partially_free_list );

#define REMOVE_FROM_PARTIAL_LIST(cache_ptr)	\
	RemoveFromList( &(cache_ptr)->partially_free_slab_list_head->partially_free_list );
void * CacheAlloc(CACHE_PTR c, UINT32 flag)
{
	VADDR ret_va = NULL;
	SpinLock(&c->slock);
	
	/*if no free buffer available try to get it from free slab or vm*/
	if ( c->free_buffer_count == 0 )
	{
		if ( c->free_slabs_count == 0 )
		{
			if ( flag & SLAB_ALLOC_NO_SLEEP )
				return NULL;
		
			/*allocate a slab by calling VM*/
			if ( AddSlabToCache(c, TRUE) == -1 )
				return NULL;
		}
		else
		{	/*allocate a slab from free list*/
			
			
			ADD_TO_INUSE_TREE( c, c->completely_free_slab_list_head );
			ADD_TO_PARTIAL_LIST( c, c->completely_free_slab_list_head );
			c->completely_free_slab_list_head = STRUCT_FROM_MEMBER( SLAB_PTR, completely_free_list, c->completely_free_slab_list_head->completely_free_list.next);
			RemoveFromList( c->completely_free_slab_list_head );
			
			c->free_buffer_count += c->slab_buffer_count;
		}
	}
	/*free buffer should be available now*/
	assert(c->free_buffer_count > 0);
	
	ret_va = GetFreeBufferFromSlab( c, c->partially_free_slab_list_head );
	assert( ret_va != NULL );
		
	SpinUnlock(&c->slock);
	return ret_va;
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
	if ( (*cache_entry)->partially_free_slab_list_head) {
		/* Get one free buffer and adjust the list. */
		ret_va = GetFreeBufferFromSlab(*cache_entry, (*cache_entry)->partially_free_slab_list_head); 
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
		free_slab = (*cache_entry)->completely_free_slab_list_head;
		/* Now remove this slab from free slab list and 
		 * insert it into in use slab tree and partially free slab list. 
		 */
		RemoveFromList(&((*cache_entry)->completely_free_slab_list_head->completely_free_list));
		InsertNodeIntoAvlTree((AVL_TREE_PTR*)&((*cache_entry)->in_use_slab_tree_root),
				(&(free_slab->in_use_tree)));
		
		AddToListTail(&((*cache_entry)->partially_free_slab_list_head->partially_free_list),
				(&(free_slab->partially_free_list)));

		free_slab->used_buffer_count = 1;
		
		/* Now get one buffer from this slab */	
		ret_va = GetFreeBufferFromSlab(*cache_entry, free_slab);
		goto FREE_BUFFER_FOUND;
	}

	/* STEP3:
	 * Try to add some slabs by requesting VM.
	 */
	if (!AddSlabToCache(*cache_entry, FALSE)) /* SUCCESS */
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
static int AddSlabToCache(CACHE_PTR c, int immediate_use)
{
	VADDR slab_start;
	SLAB_PTR s;
	/*here add conditon to check max_slabs count and exit*/
	
	slab_start = VM_ALLOC( c->slab_size );
	if ( slab_start == NULL )
		return -1;
	/*calculate the correct slab meta data and initialize it*/
	s = (SLAB_PTR) (slab_start + c->slab_metadata_offset);
	SlabInit(s, c->slab_buffer_count);
	
	/*if this slab will be consumed immediately then for performance add it directly to the use the used list*/
	if ( immediate_use )
	{
		InsertNodeIntoAvlTree( &c->in_use_slab_tree_root, &s->in_use_tree );
		AddToList( &c->partially_free_slab_list_head->partially_free_list , &s->partially_free_list );
		c->free_buffer_count += c->slab_buffer_count;
	}
	else
	{
		AddToList( &c->completely_free_slab_list_head->completely_free_list , &s->completely_free_list );
		c->free_slabs_count++;
	}
	
	return 0;
}



/*!
	\brief	Gets a free Buffer from the given slab.

	\param
		free_slab:	Pointer to the slab

	\return	 Virtual address of the free buffer.
*/
static VADDR GetFreeBufferFromSlab(CACHE_PTR c, SLAB_PTR s)
{
	VADDR ret_va = SLAB_START(s, c->slab_metadata_offset);
	UINT32 free_buffer_index = 0;
	
	/*atleast one buffer should be free*/
	assert ( s->used_buffer_count < c->slab_buffer_count );
	s->used_buffer_count++;
	
	//todo - find the first cleared bit in the bitmap
	
	
	ret_va += ( BUFFER_SIZE(c->buffer_size) * free_buffer_index);
	
	//set the bitmap to indicate it is used
	s->buffer_usage_bitmap[ free_buffer_index/sizeof(UINT32) ] |= (1<<(free_buffer_index%sizeof(UINT32)));
	
	//If all buffers utilized remove from partialy free list
	if ( s->used_buffer_count == c->slab_buffer_count )
	{
		c->partially_free_slab_list_head = STRUCT_FROM_MEMBER( SLAB_PTR, partially_free_list, s->partially_free_list.next);
		RemoveFromList( &s->partially_free_list );
	}
	
	return ret_va;
}
