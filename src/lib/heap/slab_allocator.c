/*!
  \file		src/lib/heap/slab_allocator.c
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created:	Fri Mar 21, 2008  11:30PM
  			Last modified: Fri May 23, 2008  11:10AM
  \brief	Contains functions to manage slab allocator.
*/

#include <ace.h>
#include <heap/slab_allocator.h>
#include <heap/heap.h>
#include <string.h>
#include <ds/binary_tree.h>
#include <ds/bits.h>
#include <ds/align.h>

static SLAB_ALLOCATOR_METADATA slab_alloactor_metadata;

#define VM_PAGE_SIZE	slab_alloactor_metadata.vm_page_size
#define VM_PAGE_SHIFT	slab_alloactor_metadata.vm_page_shift
#define VM_ALLOC		slab_alloactor_metadata.virtual_alloc
#define VM_FREE			slab_alloactor_metadata.virtual_free
#define VM_PROTECT		slab_alloactor_metadata.virtual_protect

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
#define SLAB_SIZE(buffer_size)		(ALIGN_UP( ((buffer_size) << 3)+sizeof(SLAB)+1 ,  VM_PAGE_SHIFT) )
/*max number of pages in the slab*/
#define SLAB_PAGES(buffer_size)		( (SLAB_SIZE(buffer_size)) >> VM_PAGE_SHIFT )
/*get the start of slab from slab metadata addresss*/
#define SLAB_START(slab_metadata_ptr, cache_ptr)	( ((UINT32)slab_metadata_ptr) - cache_ptr->slab_metadata_offset )

static void InitSlab(SLAB_PTR s, UINT32 buffer_count);
static int AddSlabToCache(CACHE_PTR cache_ptr, int immediate_use);
static VADDR GetFreeBufferFromCache(CACHE_PTR cache_ptr);
static SLAB_PTR SearchBufferInTree( VADDR buffer, CACHE_PTR cache_ptr );

/*list/tree management static functions*/
static inline SLAB_STATE GetSlabState(CACHE_PTR cache_ptr, SLAB_PTR slab_ptr);
static int ManageSlabStateTransition(CACHE_PTR cache_ptr, SLAB_PTR slab_ptr, SLAB_STATE old_state, SLAB_STATE new_state);

static void AddToPartialList(CACHE_PTR cache_ptr, SLAB_PTR slab_ptr);
static void RemoveFromPartialList(CACHE_PTR cache_ptr, SLAB_PTR slab_ptr);
static void AddToCompletelyFreeList(CACHE_PTR cache_ptr, SLAB_PTR slab_ptr);
static void RemoveFromCompletelyFreeList(CACHE_PTR cache_ptr, SLAB_PTR slab_ptr);
static void RemoveInUseTree(AVL_TREE_PTR root, CACHE_PTR cache_ptr);

static COMPARISION_RESULT slab_inuse_tree_compare(AVL_TREE_PTR node1, AVL_TREE_PTR node2);

static inline SLAB_STATE GetSlabState(CACHE_PTR cache_ptr, SLAB_PTR slab_ptr)
{
	if ( slab_ptr->used_buffer_count == 0 ) 
		return SLAB_STATE_FREE;
	else if ( slab_ptr->used_buffer_count == cache_ptr->slab_buffer_count )
		return SLAB_STATE_USED;
	else
		return SLAB_STATE_MIXED;

}
static int ManageSlabStateTransition(CACHE_PTR cache_ptr, SLAB_PTR slab_ptr, SLAB_STATE old_state, SLAB_STATE new_state)
{
	//quick check for no state change
	if ( old_state == new_state )
		return 0;
	
	//new to free
	if ( old_state == SLAB_STATE_NEW && new_state == SLAB_STATE_FREE )
	{
		cache_ptr->total_slabs++;
#ifdef SLAB_STAT_ENABLED
		if ( cache_ptr->total_slabs > cache_ptr->stat.max_slabs_used )
			cache_ptr->stat.max_slabs_used = cache_ptr->total_slabs;
#endif
		AddToCompletelyFreeList(cache_ptr, slab_ptr);
	}
	//free to mixed
	else if ( old_state == SLAB_STATE_FREE && new_state == SLAB_STATE_MIXED )
	{
		AddToPartialList(cache_ptr, slab_ptr);
		RemoveFromCompletelyFreeList(cache_ptr, slab_ptr);
		InsertNodeIntoAvlTree( &(cache_ptr->in_use_slab_tree_root), &(slab_ptr->in_use_tree) );
		cache_ptr->free_buffer_count += cache_ptr->slab_buffer_count;
	}
	//mixed to free
	else if ( old_state == SLAB_STATE_MIXED && new_state == SLAB_STATE_FREE )
	{
		RemoveFromPartialList(cache_ptr, slab_ptr);
		AddToCompletelyFreeList(cache_ptr, slab_ptr);
		RemoveNodeFromAvlTree( &(cache_ptr->in_use_slab_tree_root), &(slab_ptr->in_use_tree) );
		cache_ptr->free_buffer_count -= cache_ptr->slab_buffer_count;
	}
	//mixed to used
	else if ( old_state == SLAB_STATE_MIXED && new_state == SLAB_STATE_USED )
	{
		RemoveFromPartialList(cache_ptr, slab_ptr);
	}
	//used to mixed
	else if ( old_state == SLAB_STATE_USED && new_state == SLAB_STATE_MIXED )
	{
		AddToPartialList(cache_ptr, slab_ptr);
	}
	else
	{
		return -1;
	}
	//sucess
	return 0;
}

static void AddToPartialList(CACHE_PTR cache_ptr, SLAB_PTR slab_ptr)
{
	if ( cache_ptr->partially_free_slab_list_head != NULL)
		AddToList( &cache_ptr->partially_free_slab_list_head->partially_free_list, 
					&slab_ptr->partially_free_list );
	else
		cache_ptr->partially_free_slab_list_head = slab_ptr;
}

static void RemoveFromPartialList(CACHE_PTR cache_ptr, SLAB_PTR slab_ptr)
{
	SLAB_PTR next_slab_ptr = STRUCT_FROM_MEMBER( SLAB_PTR, partially_free_list, (slab_ptr)->partially_free_list.next);
	if ( slab_ptr == next_slab_ptr )
		cache_ptr->partially_free_slab_list_head = NULL;
	else
		cache_ptr->partially_free_slab_list_head = next_slab_ptr;
		
	RemoveFromList( &slab_ptr->partially_free_list );
}	

static void AddToCompletelyFreeList(CACHE_PTR cache_ptr, SLAB_PTR slab_ptr)
{
	if ( cache_ptr->completely_free_slab_list_head != NULL )
		AddToList( &cache_ptr->completely_free_slab_list_head->completely_free_list , &slab_ptr->completely_free_list );
	else
		cache_ptr->completely_free_slab_list_head = slab_ptr;
	
	cache_ptr->free_slabs_count++;
}

static void RemoveFromCompletelyFreeList(CACHE_PTR cache_ptr, SLAB_PTR slab_ptr)
{
	SLAB_PTR next_slab_ptr = STRUCT_FROM_MEMBER( SLAB_PTR, completely_free_list, (slab_ptr)->completely_free_list.next);
	if ( slab_ptr == next_slab_ptr )
		cache_ptr->completely_free_slab_list_head = NULL;
	else
		cache_ptr->completely_free_slab_list_head = next_slab_ptr;
		
	RemoveFromList( &slab_ptr->completely_free_list );
	cache_ptr->free_slabs_count--;
}

/*!
	\brief	 Compares the addresses and returns if greater-than/lesser-than or equal to accordingly.
	\param	node1 - Pointer to an AVL Tree node
	\param	node2 - Pointer to an AVL Tree node
	\return
		LESS_THAN:	If virtual address of node1 < node2
		GREATER_THAN: If virtual address of node1 > node2
		EQUAL: If virtual address of node1 = node2
*/
static COMPARISION_RESULT slab_inuse_tree_compare(AVL_TREE_PTR node1, AVL_TREE_PTR node2)
{
		if ( node1 < node2 )
		{
			return GREATER_THAN;
		}
		else if ( node1 > node2 )
		{
			return LESS_THAN;
		}
		else
		{
			return EQUAL;
		}
}

/*!
	\brief	Initializes the contents of a slab.
	\param	slab_ptr - Pointer to slab that has to be initialized.
	\param	buffer_count - count of buffers in this slab. 
	\return	 void 
*/
static void InitSlab(SLAB_PTR slab_ptr, UINT32 buffer_count)
{
	int nbytes;
	/*initialize the tree and list */
	InitList( &(slab_ptr->partially_free_list) );
	InitAvlTreeNode( &(slab_ptr->in_use_tree), slab_inuse_tree_compare);
	InitList( &(slab_ptr->completely_free_list) );
	
	/*todo - call the constructor on each buffer*/
	
	/*all buffers are free*/
	slab_ptr->used_buffer_count = 0;
	nbytes = buffer_count / BITS_PER_BYTE;
	if ( buffer_count % BITS_PER_BYTE )
	{
		nbytes++;
	}
	memset( slab_ptr->buffer_usage_bitmap, 0, nbytes);
	return;
}

/*!
	\brief	Initializes a slab allocator. This is an 1 time operation.
	\param	page_size -  Size of virtual page.
	\param	v_alloc - Function Pointer to virtual alloc.
	\param	v_free - Function pointer to virtual free.
	\param 	v_protect -	Function pointer to virtual protect.
	\return	 0 on sucess and -1 on failure
*/
int InitSlabAllocator(UINT32 page_size, void * (*v_alloc)(int size), 
	int (*v_free)(void * va, int size),
	int (*v_protect)(void * va, int size, int protection)  )
{
	VM_PAGE_SIZE = page_size;
	if ( FindFirstSetBitInBitArray( &VM_PAGE_SIZE, sizeof(VM_PAGE_SIZE) * BITS_PER_BYTE, &VM_PAGE_SHIFT) == -1 )
		return -1;
	VM_ALLOC = v_alloc;
	VM_FREE = v_free;
	VM_PROTECT = v_protect;
	return 0;
}


/*!
	\brief	 Initializes an empty cache of specified buffer size.
	\param	new_cache - A static cache created in data segment.
	\param	size - size of the buffers in cache.
	\param	free_slabs_threshold - Threshold to start VM operation.
	\param	min_slabs - Minimum no of slabs to be present always.
	\param	max_slabs - Maximum no of slabs allowed.
	\param	constructor - Function pointer which initializes the newly created slab.
	\param	destructor - Function pointer which reuses a slab.
	\return	
		Success(0) if cache is created.
		Failure(-1) if cache is not created.
*/
int InitCache(CACHE_PTR new_cache, UINT32 size,
		int free_slabs_threshold, int min_buffers, int max_slabs,
		int (*constructor)(void *buffer), 
		int (*destructor)(void *buffer))
{
	int buf_count, bitmap_size;
	
	if ( size <= 0 )
		return -1;

	InitSpinLock( &new_cache->slock);

	new_cache->buffer_size = BUFFER_SIZE(size);
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
		
	new_cache->slab_size = SLAB_SIZE(new_cache->buffer_size);
	buf_count = (new_cache->slab_size - sizeof(SLAB)) / new_cache->buffer_size;
	bitmap_size = buf_count / BITS_PER_BYTE;
	/*recalcualte the buffer count*/
	buf_count = (new_cache->slab_size - sizeof(SLAB) - bitmap_size) / new_cache->buffer_size;
	
	new_cache->slab_metadata_size = sizeof(SLAB) + buf_count/BITS_PER_BYTE;
	//align the size
	new_cache->slab_metadata_size = ALIGN_UP(new_cache->slab_metadata_size, 2);
	new_cache->slab_metadata_offset  = new_cache->slab_size - new_cache->slab_metadata_size;
	new_cache->slab_buffer_count = buf_count;
	
#ifdef SLAB_STAT_ENABLED
	new_cache->stat.alloc_calls = 0; 
	new_cache->stat.free_calls = 0;
	
	new_cache->stat.vm_alloc_calls = 0;
	new_cache->stat.vm_free_calls = 0;
	
	new_cache->stat.alloc_failures = 0;
	
	new_cache->stat.max_slabs_used = 0;
	new_cache->stat.average_slab_usage = 0;
#endif

	
	return 0;
}

/*!
	\brief	Gets a free buffer from cache. 
	\param	cache_ptr - Pointer to cache from which buffers are wanted.
	\param 	flag - To indicate if this function can sleep(0) or not(1).
	\return
		On Success: Virtual address of a free buffer.
		On Failure: NULL
*/
void* GetVAFromCache(CACHE_PTR cache_ptr, UINT32 flag)
{
	VADDR ret_va = NULL;
	SpinLock(&(cache_ptr->slock));

#ifdef SLAB_STAT_ENABLED
	cache_ptr->stat.alloc_calls++;
#endif
	/* If no free buffer is available, try to get it from free slab */
	if ( cache_ptr->free_buffer_count == 0 )
	{
		/* if no free slab list get it from VM*/
		if ( cache_ptr->free_slabs_count == 0 )
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
	}
	
	ret_va = GetFreeBufferFromCache( cache_ptr );
	assert( ret_va );
		
FINDING_BUFFER_DONE:

#ifdef SLAB_STAT_ENABLED
	if ( ret_va == NULL )
		cache_ptr->stat.alloc_failures++;
#endif

	SpinUnlock(&(cache_ptr->slock));
	return (void*)(ret_va);
}

/*!
	\brief	 Adds slabs to cache by requesting memory from VM.
	\param	cache_ptr - Pointer to my cache entry.
	\param	immediate_use- Are you using a free buffer from the new slab immediately?
	\return
		0	if successfully fetched from VM 
		-1 	if failure.
	\note 	Holds a lock to cache pointer.
	\todo - for performance the flag immediate use can be used as hint...
*/
static int AddSlabToCache(CACHE_PTR cache_ptr, int immediate_use)
{
	VADDR slab_start;
	SLAB_PTR slab_ptr;
	/* TBD. Add conditon to check max_slabs count and exit */

#ifdef SLAB_STAT_ENABLED
	cache_ptr->stat.vm_alloc_calls++;
#endif
	slab_start = (VADDR) VM_ALLOC( cache_ptr->slab_size );
	if ( slab_start == NULL )
	{
		return -1;
	}

	/*calculate the correct slab meta data and initialize it*/
	slab_ptr = (SLAB_PTR) (slab_start + cache_ptr->slab_metadata_offset);
	InitSlab(slab_ptr, cache_ptr->slab_buffer_count);
	
	ManageSlabStateTransition( cache_ptr, slab_ptr, SLAB_STATE_NEW, SLAB_STATE_FREE );
	
	return 0;
}

/*!
	\brief	Gets a free Buffer from the given slab.
	\param	slab_ptr - Pointer to the slab from which a free buffer is wanted.
	\param	cache_ptr - Pointer to cache which has the free buffer.
	\return	 Virtual address of the free buffer.
	\note		Hold a lock to cache pointer.
*/
static VADDR GetFreeBufferFromCache(CACHE_PTR cache_ptr)
{
	SLAB_PTR slab_ptr;
	SLAB_STATE old_state, new_state;
	VADDR ret_va;
	UINT32 free_buffer_index = 0;
	
	slab_ptr = cache_ptr->partially_free_slab_list_head;
	/*if partial free list is empty, get it from completely free list*/
	if ( slab_ptr == NULL )
		slab_ptr = cache_ptr->completely_free_slab_list_head;
	
	/*atleast one buffer should be free*/
	assert( slab_ptr );
	assert ( slab_ptr->used_buffer_count < cache_ptr->slab_buffer_count );
	
	/*move the slab in/out of the differnt lists/tree*/
	old_state = GetSlabState( cache_ptr, slab_ptr);
	slab_ptr->used_buffer_count++;
	new_state = GetSlabState( cache_ptr, slab_ptr);
	ManageSlabStateTransition( cache_ptr, slab_ptr, old_state, new_state );
	
	/*Find the first free buffer*/
	if ( FindFirstClearBitInBitArray((void*)(slab_ptr->buffer_usage_bitmap), cache_ptr->slab_buffer_count, &free_buffer_index) == -1 )
		return NULL;
	
	/* Set the bitmap to indicate the buffer is used */
	SetBitInBitArray((void*)(slab_ptr->buffer_usage_bitmap), free_buffer_index );
	
	cache_ptr->free_buffer_count --;
	
	/*calculate the virtual address*/
	ret_va = SLAB_START(slab_ptr, cache_ptr) + ( cache_ptr->buffer_size * free_buffer_index);
	
	return ret_va;
}

/*!
	\brief	 Free A buffer in it's slab. If all buffers in the slab are free, move the slab to completely free slab list.
	\param	buffer - Pointer to buffer which is to be freed.
	\param	cache_ptr-	Pointer to cache which contans the buffer.
	\return
		0:	Success; If freed successfully.
		-1:	Failure; If given buffer isn't found in the cache.
	\note		Holds a lock to cache_ptr
*/
int FreeBuffer(void *buffer, CACHE_PTR cache_ptr)
{
	SLAB_PTR slab_ptr;
	int buffer_index;
	VADDR va_start;
	SLAB_STATE old_state, new_state;
	char byte;

#ifdef SLAB_STAT_ENABLED
	cache_ptr->stat.free_calls++;
#endif

	if (buffer == NULL || cache_ptr == NULL)
	{
		return -1;
	}

	/* Find the slab which contains this buffer, using in_use_slab_tree */
	slab_ptr = SearchBufferInTree( (VADDR)(buffer), cache_ptr);
	if ( slab_ptr == NULL )
	{
		printf("slab not found in tree\n");
		return -1;
	}

	/* Clear the corresponding bit in buffer_usage_bitmap */
	va_start = SLAB_START(slab_ptr, cache_ptr);
	buffer_index = ( ((VADDR)buffer - va_start) / (cache_ptr->buffer_size) );
	/* see if this buffer is presently used */ 
	byte = GetBitFromBitArray( (void*)(slab_ptr->buffer_usage_bitmap), buffer_index );
	if(byte == 0)
	{
		printf("I can't free a buffer which is not allocated %d!\n", buffer_index);
		return -1;
	}

	ClearBitInBitArray( (void*)(slab_ptr->buffer_usage_bitmap), buffer_index );	
	
	old_state = GetSlabState( cache_ptr, slab_ptr);
	slab_ptr->used_buffer_count --;
	cache_ptr->free_buffer_count ++;
	new_state = GetSlabState( cache_ptr, slab_ptr);
	
	ManageSlabStateTransition( cache_ptr, slab_ptr, old_state, new_state );
	
	/* If free buffer count is greater than free_slabs_threshold, then start VM operation */
	// TBD
	return 0;
}


/*!
	\brief	Remove the entire in_use_tree 

	\param	root: This is the root of the tree that is to be removed.
			cache_ptr:	Pointer to cache which contains the tree to be removed.

	\return	void
*/
static void RemoveInUseTree(AVL_TREE_PTR root, CACHE_PTR cache_ptr)
{
	if(root)
	{
		SLAB_PTR slab_ptr;
		int i, count;

		if(!IS_AVL_TREE_LEFT_LIST_END(root))
			RemoveInUseTree(AVL_TREE_LEFT_NODE(root), cache_ptr);
		if(!IS_AVL_TREE_RIGHT_LIST_END(root))
			RemoveInUseTree(AVL_TREE_RIGHT_NODE(root), cache_ptr);
		
		RemoveNodeFromAvlTree( &(cache_ptr->in_use_slab_tree_root), root );
		slab_ptr = STRUCT_FROM_MEMBER( SLAB_PTR, in_use_tree, root);
		cache_ptr->free_buffer_count -= (cache_ptr->slab_buffer_count - slab_ptr->used_buffer_count);
		/*clear the buffer usage bitmap */
		for(i=0, count=slab_ptr->used_buffer_count; count >=0; i++, count -= 8)
			slab_ptr->buffer_usage_bitmap[i] = 0;

		slab_ptr->used_buffer_count = 0;
		AddToCompletelyFreeList(cache_ptr, slab_ptr);
	}
}


/*!
	\brief	Destroy the cache and return the vm_pages to VM subsystem. 
	\param	cache_ptr: Pointer to cache which is to be destroyed.
	\return	void
*/
void DestroyCache(CACHE_PTR rem_cache)
{
	UINT32 free_slabs;
	SLAB_PTR slab_ptr;
	VADDR rem_va;

	/* Get a lock to cache */
	SpinLock( &(rem_cache->slock) );

	/*Move all partially used slabs to compeletely free slabs list*/
	slab_ptr = rem_cache->partially_free_slab_list_head;
	while( slab_ptr != NULL )
	{
		SLAB_STATE old_state = GetSlabState( rem_cache, slab_ptr);
		
		rem_cache->free_buffer_count += slab_ptr->used_buffer_count;
		slab_ptr->used_buffer_count = 0;
		
		ManageSlabStateTransition( rem_cache, slab_ptr, old_state, GetSlabState(rem_cache, slab_ptr) );
		
		/*get next partially used slab*/
		slab_ptr = rem_cache->partially_free_slab_list_head;
	}

	/* Now remove all slabs from completely FULL slab list.
	 * This is possible by deleting all nodes from the in use tree.
	 */
	RemoveInUseTree(rem_cache->in_use_slab_tree_root, rem_cache);
	
	/* Before proceeding, make sure this cache is no more used by anybody */
	assert( rem_cache->in_use_slab_tree_root == NULL);

	/* Now all slabs are in completely free list */
	/* Free the vm_pages inside slabs pointed by completely free slab list and the slabs themselves.*/
	free_slabs = rem_cache->free_slabs_count;
	while( free_slabs )
	{
		slab_ptr =  rem_cache->completely_free_slab_list_head;
		rem_cache->completely_free_slab_list_head = STRUCT_FROM_MEMBER( SLAB_PTR, completely_free_list, rem_cache->completely_free_slab_list_head->completely_free_list.next);
		RemoveFromCompletelyFreeList( rem_cache, slab_ptr );
		
		/* Now get the starting address of slab */
		rem_va = SLAB_START( slab_ptr, rem_cache);

#ifdef SLAB_STAT_ENABLED
		rem_cache->stat.free_calls++;
#endif
		VM_FREE( (void*)rem_va, rem_cache->slab_size );
		free_slabs--;
	}
	SpinUnlock( &(rem_cache->slock) );
	return;
}

/*!
	\brief	 Finds the slab in tree, which contains the given buffer.
	\param	buffer - The Free memory that has to be released to it's slab.
			cache_ptr - Pointer to cache which contains the given buffer.
	\return
   		On SUCCESS: Returns the slab pointer which contains the buffer.
		On Failure: Returns NULL.
*/
static SLAB_PTR SearchBufferInTree( VADDR buffer, CACHE_PTR cache_ptr )
{
	AVL_TREE_PTR root;
	VADDR start_va;
	SLAB_PTR slab_ptr;

 	/* Without a valid buffer and cache_ptr we can't do anything */
	if (!cache_ptr || !buffer)
	{
		return NULL;
	}

	root = cache_ptr->in_use_slab_tree_root;
	while ( root )
	{
		slab_ptr = STRUCT_FROM_MEMBER( SLAB_PTR, in_use_tree, root);
		start_va = (VADDR) SLAB_START( slab_ptr, cache_ptr);
		if ( buffer >= start_va && buffer < (start_va + cache_ptr->slab_metadata_offset) )
		{
			return slab_ptr;
		}
		else if ( buffer < start_va )
		{
			if ( IS_AVL_TREE_LEFT_LIST_END(root) )
				return NULL;
			root = AVL_TREE_LEFT_NODE(root);
		}
		else //if ( buffer > (start_va + cache_ptr->slab_metadata_offset) )
		{
			if ( IS_AVL_TREE_RIGHT_LIST_END(root) )
				return NULL;
			root = AVL_TREE_RIGHT_NODE(root);
		}
	}
	return NULL;
}
/*!
	\brief	returns the cache statistics structure pointer
	\param	cache_ptr - Pointer to cache for which stats are required
	\return
   		On SUCCESS: Returns the cache statistics structure pointer
		On Failure(if stats are not enabled): Returns NULL.
*/
CACHE_STATISTICS_PTR GetCahcheStatistics(CACHE_PTR cache_ptr)
{
	CACHE_STATISTICS_PTR ret = NULL;
#ifdef SLAB_STAT_ENABLED
	ret = &cache_ptr->stat;
#endif
	return ret;
}
