/*!
  \file		src/lib/heap/slab_allocator.c
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created:	Fri Mar 21, 2008  11:30PM
  			Last modified: Mon Apr 07, 2008  01:36PM
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


#define ADD_TO_INUSE_TREE(cache_ptr, slab_ptr)	\
	InsertNodeIntoAvlTree( &(cache_ptr)->in_use_slab_tree_root, &(slab_ptr)->in_use_tree );
	
#define REMOVE_FROM_INUSE_TREE(cache_ptr, slab_ptr)	\
	RemoveNodeFromAvlTree( &(cache_ptr)->in_use_slab_tree_root, &(slab_ptr)->in_use_tree );
	
#define ADD_TO_PARTIAL_LIST(cache_ptr, slab_ptr)	\
	AddToList( &(cache_ptr)->partially_free_slab_list_head->partially_free_list , &(slab_ptr)->partially_free_list );

#define REMOVE_FROM_PARTIAL_LIST(cache_ptr)	\
	RemoveFromList( &(cache_ptr)->partially_free_slab_list_head->partially_free_list );


typedef struct search_slab
{
	AVL_TREE	tree;
	VADDR		va_start;
	UINT32		va_size;
} SEARCH_SLAB, *SEARCH_SLAB_PTR;


static COMPARISION_RESULT slab_inuse_tree_compare(AVL_TREE_PTR a, AVL_TREE_PTR b);
static int GetSlabMetadataInfo(UINT32 buffer_size, UINT32 * slab_total_size, UINT32 * metadata_size, UINT32 * metadata_offset);
static void InitSlab(SLAB_PTR s, UINT32 buffer_count);
static int AddSlabToCache(CACHE_PTR cache_ptr, int immediate_use);
static VADDR GetFreeBufferFromSlab(CACHE_PTR cache_ptr, SLAB_PTR slab_ptr);


static COMPARISION_RESULT slab_inuse_tree_compare(AVL_TREE_PTR a, AVL_TREE_PTR b)
{
	return 0;
}

/*!
	\brief	Calculates and returns the information about the slab metadata for a given buffer size

	\param
   		buffer_size: Size of buffer
		slab_tottal_size: size of buffers in slab + size of meta data + slize of bitmap	
		metadata_size: Size of slab meta data
		metadata_offset: Offset of metatadata from the slab start.

	\return	Maximum number of buffers in the slab.

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



/*!
	\brief	Initializes the contents of a slab.

	\param
		slab_ptr: Pointer to slab that has to be initialized.
		buffer_count: 

	\return	 void 
*/
static void InitSlab(SLAB_PTR slab_ptr, UINT32 buffer_count)
{
	/*initialize the tree and list */
	InitList( &(slab_ptr->partially_free_list) );
	InitAvlTreeNode( &(slab_ptrs->in_use_tree), slab_inuse_tree_compare);
	InitList( &(slab_ptr->completely_free_list) );
	
	/*all buffers are free*/
	slab_ptr->used_buffer_count = 0;
	memset( slab_ptr->buffer_usage_bitmap, 0, buffer_count/BITS_PER_BYTE);
	return;
}



/*!
	\brief: Initializes a slab allocator. This is an 1 time operation.

	\param
		page_size: Size of virtual page.
		v_alloc: Function Pointer to virtual alloc.
		v_free:	Function pointer to virtual free.
		v_protect:	Function pointer to virtual protect.

	\return	 void
*/
void InitSlabAllocator(UINT32 page_size, void * (*v_alloc)(int size), 
	void * (*v_free)(void * va, int size),
	void * (*v_protect)(void * va, int size, int protection)  )
{
	VM_PAGE_SIZE = page_size;
	VM_ALLOC = v_alloc;
	VM_FREE = v_free;
	VM_PROTECT = v_protect;
	return;
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
int InitCache(CACHE_PTR new_cache, UINT32 size,
		int free_slabs_threshold, int min_buffers, int max_slabs,
		int (*constructor)(void *buffer), 
		int (*destructor)(void *buffer))
{
	InitSpinLock( &new_cache->slock);

	new_cache->buffer_size = size;
	new_cache->constructor = constructor;
	new_cache->destructor = destructor;
	
	new_cache->min_buffers = min_buffers;
	new_cache->max_slabs = max_slabs;
	new_cache->free_slabs_threshold = free_slabs_threshold;
	
	new_cache->partially_free_slab_list_head = NULL;
	new_cache->in_use_slab_tree_root = NULL;
	
	new_cache->completely_free_slab_list_head = NULL;
	new_cache->free_slabs_count = 0;
	
	new_cache->free_buffer_count = 0;
	
	new_cache->slab_buffer_count = GetSlabMetadataInfo(size, &new_cache->slab_size, &new_cache->slab_metadata_size, &new_cache->slab_metadata_offset);
	
	return 0;
}



/*!
	\brief	Gets a free buffer from cache. 

	\param
		cache_ptr: Pointer to cache from which buffers are wanted.
		flag: To indicate if this function can sleep or not.

	\return
		On Success: Virtual address of a free buffer.
		On Failure: NULL
*/
void* GetVAFromCache(CACHE_PTR cache_ptr, UINT32 flag)
{
	VADDR ret_va = NULL;
	SpinLock(&(cache_ptr->slock));
	
	/* If no free buffer is available, try to get it from free slab or vm */
	if ( 0 == cache_ptr->free_buffer_count )
	{
		/* Try to get from Free Slab list */
		if ( 0 == cache_ptr->free_slabs_count )
		{
			/* If allowed to sleep, try to get from VM or else return failure */
			if ( flag & CACHE_ALLOC_NO_SLEEP )
			{
				goto FINDING_BUFFER_DONE;
			}
		
			/* Allocate a slab by calling VM */
			if ( AddSlabToCache(cache_ptr, TRUE) == -1 )
			{
				goto FINDING_BUFFER_DONE;
			}
				
		}
		else
		{	
			/*allocate a slab from free list*/
			ADD_TO_INUSE_TREE( cache_ptr, cache_ptr->completely_free_slab_list_head );
			ADD_TO_PARTIAL_LIST( cache_ptr, cache_ptr->completely_free_slab_list_head );
			cache_ptr->free_buffer_count += cache_ptr->slab_buffer_count;

			if ( 1 == cache_ptr->free_slabs_count )
			{
				/* I am the last slab inside the cache */
				cache_ptr->completely_free_slab_list_head = NULL;
			}
			else
			{
				cache_ptr->completely_free_slab_list_head = STRUCT_FROM_MEMBER( SLAB_PTR, completely_free_list, cache_ptr->completely_free_slab_list_head->completely_free_list.next);
				RemoveFromList( cache_ptr->completely_free_slab_list_head );
			}
			cache_ptr->free_slabs_count--;
		}
	}
	/*free buffer should be available now*/
	assert(cache_ptr->free_buffer_count > 0);
	
	ret_va = GetFreeBufferFromSlab( cache_ptr, cache_ptr->partially_free_slab_list_head );
	assert( NULL != ret_va );
		
FINDING_BUFFER_DONE:
	SpinUnlock(&(cache_ptr->slock));
	return ret_va;
}


/*!
	\brief	 Adds slabs to cache by requesting memory from VM.

	\param
		cache_ptr: Pointer to my cache entry.
		immediate_use: Are you using a free buffer from the new slab immediately?

	\return
		0	if successfully fetched from VM 
		-1 	if failure.

	\assumptions: Holds a lock to cache pointer.
*/
static int AddSlabToCache(CACHE_PTR cache_ptr, int immediate_use)
{
	VADDR slab_start;
	SLAB_PTR slab_ptr;
	/* TBD. Add conditon to check max_slabs count and exit */
	
	slab_start = VM_ALLOC( cache_ptr->slab_size );
	if ( NULL == slab_start )
	{
		return -1;
	}

	/*calculate the correct slab meta data and initialize it*/
	slab_ptr = (SLAB_PTR) (slab_start + cache_ptr->slab_metadata_offset);
	InitSlab(slab_ptr, cache_ptr->slab_buffer_count);
	
	/*if this slab will be consumed immediately then for performance add it directly to the used list*/
	if ( immediate_use )
	{
		InsertNodeIntoAvlTree( &(cache_ptr->in_use_slab_tree_root), &(slab_ptr->in_use_tree) );
		AddToList( &(cache_ptr->partially_free_slab_list_head->partially_free_list) , &(slab_ptr->partially_free_list) );
		cache_ptr->free_buffer_count += cache_ptr->slab_buffer_count;
	}
	else
	{
		AddToList( &(cache_ptr->completely_free_slab_list_head->completely_free_list) , &(slab_ptr->completely_free_list) );
		cacke_ptr->free_slabs_count++;
	}
	
	return 0;
}



/*!
	\brief	Gets a free Buffer from the given slab.

	\param
		slab_ptr: Pointer to the slab from which a free buffer is wanted.
		cache_ptr: Pointer to cache which has the free buffer.

	\return	 Virtual address of the free buffer.
	
	\assumptions: Hold a lock to cache pointer.
*/
static VADDR GetFreeBufferFromSlab(CACHE_PTR cache_ptr, SLAB_PTR slab_ptr)
{
	VADDR ret_va = SLAB_START(slab_ptr, cache_ptr->slab_metadata_offset);
	UINT32 free_buffer_index = 0;
	
	/*atleast one buffer should be free*/
	assert ( slab_ptr->used_buffer_count < cache_ptr->slab_buffer_count );
	slab_ptr->used_buffer_count++;
	cache_ptr->free_buffer_count--;
	
	/* TBD. BIT_ARRAY_FIND_FIRST_CLEAR_BIT(slab_ptr->buffer_usage_bitmap, 32, free_buffer_index); */
	
	ret_va += ( BUFFER_SIZE(cache_ptr->buffer_size) * free_buffer_index);
	
	/* Set the bitmap to indicate it is used */
	slab_ptr->buffer_usage_bitmap[ free_buffer_index/sizeof(UINT32) ] |= (1<<(free_buffer_index%sizeof(UINT32)));
	/* TBD. Replace the above l;ine with below line */
	/* BIT_ARRAY_SET_BIT( slab_ptr->buffer_usage_bitmap, free_buffer_index ); */
	
	/* If all buffers utilized remove from partialy free list */
	if ( slab_ptr->used_buffer_count == cache_ptr->slab_buffer_count )
	{
		cache_ptr->partially_free_slab_list_head = STRUCT_FROM_MEMBER( SLAB_PTR, partially_free_list, slab_ptr->partially_free_list.next);
		RemoveFromList( &(slab_ptr->partially_free_list) );
	}
	
	return ret_va;
}


/*!
	\brief	 Free A buffer in it's slab. If all buffers in the slab are free, move the slab to completely free slab list.

	\param
		buffer: Pointer to buffer which is to be freed.
		cache_ptr:	Pointer to cache which contans the buffer.

	\return
		0:	Success; If freed successfully.
		-1:	Failure; If given buffer isn't found in the cache.

	\assumptions	Holds a lock to cache_ptr
*/
int FreeBuffer(void *buffer, CACHE_PTR cache_ptr)
{
	SEARCH_SLAB temp_slab;
	AVL_TREE tree_ptr;
	SLAB_PTR slab_ptr;
	int buffer_index, first_clear_bit, bit_array_size_in_bytes;
	VADDR va_start;

	/* Find the slab which contains this buffer using in_use_slab_tree */

	/* We have to first build an AVL TREE node */
	InitAvlTreeNode( &(temp_slab.tree), CompareSlab );
	temp_slab.va_start = buffer;
	temp_slab.va_size = cache_ptr->slab_size;

	tree_ptr = SearchAvlTree( cache_ptr->in_use_slab_tree_root, &(temp_slab.tree));

	if( NULL == tree_ptr ) /* Unable to find the buffer in given cache */
	{
		return -1;
	}

	/* Now get the slab pointer from tree pointer */
	slab_ptr = STRUCT_FROM_MEMBER( SLAB_PTR, in_use_tree, tree_ptr);

	/* Clear the corresponding bit in buffer_usage_bitmap */
	va_start = SLAB_START(slab_ptr, cache_ptr->slab_metadata_size);
	buffer_index = ( (buffer - va_start) / (cache_ptr->buffer_size) ) - 1;
	BIT_ARRAY_CLEAR_BIT( slab_ptr->buffer_usage_bitmap, buffer_index );	

	slab_ptr->used_buffer_count --;
	cache_ptr->free_buffer_count ++;

	/* If all buffers in the slab are free, move the slab to completely free slab list
	bit_array_size_in_bytes = cache_ptr->slab_buffer_count / BITS_PER_BYTE + 1;
	if ( 0 == (cache_ptr->slab_buffer_count % BITS_PER_BYTE) )
	{
		bit_array_size_in_bytes--;
	}

	BIT_ARRAY_FIND_FIRST_CLEAR_BIT(slab_ptr->buffer_usage_bitmap, bit_array_size_in_bytes, first_clear_bit);
	if ( -1 != first_clear_bit )
	{
		RemoveFromList( &(slab_ptr->partially_free_list) );
		RemoveNodeFromAvlTree( &(cache_ptr->in_use_slab_tree_root), &(slab_ptr->in_use_tree) );
		AddToList( &(cache_ptr->completely_free_slab_list_head->completely_free_list) , &(slab_ptr->completely_free_list) );
		cache_ptr->free_slabs_count ++;
		// If free buffer count is greater than free_slabs_threshold, then start VM operation
		// TBD
	}
	*/
	return 0;
}



/*!
	\brief	Destroy a cache and return the vm_pages t to VM subsystem. 

	\param
		cache_ptr: Pointer to cache which is to be destroyed.

	\return	void
*/
void DestroyCache(CACHE_PTR rem_cache)
{
	UINT32 free_slabs;
	SLAB_PTR rem_slab;
	VADDR rem_va;

	/* Get a lock to cache */
	SpinLock( &(rem_cache->slock) );

	/* Before proceeding, make sure this cache is no more used by anybody */
	assert( NULL==in_use_slab_tree_root && NULL==partially_free_slab_list_head );

	/* Free the vm_pages inside slabs pointed by completely free slab list */
	free_slabs = rem_cache->free_slabs_count;
	while( free_slabs )
	{
		rem_slab =  rem_cache->completely_free_slab_list_head;
		rem_cache->completely_free_slab_list_head = STRUCT_FROM_MEMBER( SLAB_PTR, completely_free_list, cache_ptr->completely_free_slab_list_head->completely_free_list.next);
		RemoveFromList( &(rem_slab->completely_free_list) );
		
		/* Now get the starting address of slab */
		rem_va = SLAB_START( rem_slab, rem_cache->slab_metadata_offset);
		VM_FREE( (void*)rem_va, rem_cache->slab_size );
		free_slabs--;
	}
	return;
}

COMPARISON_RESULT CompareSlab( AVL_TREE_PTR node1, AVL_TREE_PTR node2 )
{
	VADDR va_in_tree, va_to_search;
	UINT32 va_size;

	assert( node1 != NULL && node2 != NULL);

	/* Get slab pointers from tree pointers. */
	
	va_in_tree = (VADDR) (STRUCT_FROM_MEMBER( SLAB_PTR, in_use_tree, node1));
	va_to_search = (STRUCT_FROM_MEMBER( SEARCH_SLAB_PTR, tree, node2))->va_start;
	va_size = (STRUCT_FROM_MEMBER( SEARCH_SLAB_PTR, tree, node2))->va_size;

	/* now find if va_to_search is in the range (va_in_tree) to (va_in_tree + va_size) */
	if ( va_to_search < va_in_tree )
	{
		return LESS_THAN;
	}
	else if ( va_to_search > (va_in_tree + va_size) )
	{
		return GREATER_THAN;
	}
	else 
	{
		return EQUAL;
	}
}
