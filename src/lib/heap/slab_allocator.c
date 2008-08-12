/*!
  \file		src/lib/heap/slab_allocator.c
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created:	Fri Mar 21, 2008  11:30PM
  			Last modified: Wed Aug 13, 2008  12:10AM
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

/*!
 * max size of the slab(buffers+metadata)
 *	1) slab should contain atleast 8 buffer
 *	2) slab size includes its meta data
 *		a) size of the the slab structure
 *		b) size of the bitmap at the end of the slab strucutre
*/
#define SLAB_SIZE(buffer_size)		(ALIGN_UP( ((buffer_size) << 3)+sizeof(SLAB)+1 ,  VM_PAGE_SHIFT) )

/*! /def SLAB_PAGES(buffer_size)
 *	/brief	max number of pages in the slab
 */
#define SLAB_PAGES(buffer_size)		( (SLAB_SIZE(buffer_size)) >> VM_PAGE_SHIFT )

/* get the start of slab from slab metadata addresss */
#define SLAB_START(slab_metadata_ptr, cache_ptr)	( ((UINT32)slab_metadata_ptr) - cache_ptr->slab_metadata_offset )

static void InitSlab(SLAB_PTR s, UINT32 buffer_count);
static int AllocateSlabToCache(CACHE_PTR cache_ptr, int immediate_use);
static VADDR GetFreeBufferFromCache(CACHE_PTR cache_ptr);
static SLAB_PTR SearchBufferInTree( VADDR buffer, CACHE_PTR cache_ptr );

/*!	list/tree management static functions */
static inline SLAB_STATE GetSlabState(CACHE_PTR cache_ptr, SLAB_PTR slab_ptr);
static int ManageSlabStateTransition(CACHE_PTR cache_ptr, SLAB_PTR slab_ptr, SLAB_STATE old_state, SLAB_STATE new_state);

static void AddToPartialList(CACHE_PTR cache_ptr, SLAB_PTR slab_ptr);
static void RemoveFromPartialList(CACHE_PTR cache_ptr, SLAB_PTR slab_ptr);
static void AddToCompletelyFreeList(CACHE_PTR cache_ptr, SLAB_PTR slab_ptr);
static void RemoveFromCompletelyFreeList(CACHE_PTR cache_ptr, SLAB_PTR slab_ptr);
static void RemoveInUseTree(CACHE_PTR cache_ptr);

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
	/*!	quick check for no state change */
	if ( old_state == new_state )
		return 0;
	
	/*!	new to free */
	if ( old_state == SLAB_STATE_NEW && new_state == SLAB_STATE_FREE )
	{
		cache_ptr->total_slabs++;
#ifdef SLAB_STAT_ENABLED
		if ( cache_ptr->total_slabs > cache_ptr->stat.max_slabs_used )
			cache_ptr->stat.max_slabs_used = cache_ptr->total_slabs;
#endif
		AddToCompletelyFreeList(cache_ptr, slab_ptr);
	}
	/*!	free to mixed */
	else if ( old_state == SLAB_STATE_FREE && new_state == SLAB_STATE_MIXED )
	{
		AddToPartialList(cache_ptr, slab_ptr);
		RemoveFromCompletelyFreeList(cache_ptr, slab_ptr);
		InsertNodeIntoAvlTree( &(cache_ptr->in_use_slab_tree_root), &(slab_ptr->in_use_tree), 0, slab_inuse_tree_compare );
		cache_ptr->free_buffer_count += cache_ptr->slab_buffer_count;
	}
	/*! mixed to free */
	else if ( old_state == SLAB_STATE_MIXED && new_state == SLAB_STATE_FREE )
	{
		RemoveFromPartialList(cache_ptr, slab_ptr);
		AddToCompletelyFreeList(cache_ptr, slab_ptr);
		RemoveNodeFromAvlTree( &(cache_ptr->in_use_slab_tree_root), &(slab_ptr->in_use_tree), 0, slab_inuse_tree_compare );
		cache_ptr->free_buffer_count -= cache_ptr->slab_buffer_count;
	}
	/*! mixed to used */
	else if ( old_state == SLAB_STATE_MIXED && new_state == SLAB_STATE_USED )
	{
		RemoveFromPartialList(cache_ptr, slab_ptr);
	}
	/*! used to mixed */
	else if ( old_state == SLAB_STATE_USED && new_state == SLAB_STATE_MIXED )
	{
		AddToPartialList(cache_ptr, slab_ptr);
	}
	else
	{
		return -1;
	}
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
	SLAB_PTR next_slab_ptr = STRUCT_ADDRESS_FROM_MEMBER( (slab_ptr)->partially_free_list.next, SLAB, partially_free_list);
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
	SLAB_PTR next_slab_ptr = STRUCT_ADDRESS_FROM_MEMBER( (slab_ptr)->completely_free_list.next, SLAB, completely_free_list);
	if ( slab_ptr == next_slab_ptr )
		cache_ptr->completely_free_slab_list_head = NULL;
	else
		cache_ptr->completely_free_slab_list_head = next_slab_ptr;
		
	RemoveFromList( &slab_ptr->completely_free_list );
	cache_ptr->free_slabs_count--;
}

/*!
 *  \fn		slab_inuse_tree_compare
 *	\brief					Compares the addresses and returns if greater-than/lesser-than or equal to accordingly.
 *	\param	node1			Pointer to an AVL Tree node
 *	\param	node2			Pointer to an AVL Tree node
 *	\retval LESS_THAN		If virtual address of \a node1 < \a node2
 *	\retval	GREATER_THAN	If virtual address of \a node1 > \a node2
 *	\retval	EQUAL		 	If virtual address of \a node1 = \a node2
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
 *  \fn		SearchBufferInTree
 *	\brief				Finds the slab in tree, which contains the given buffer.
 *	\param	buffer		The Free memory that has to be released to it's slab.
 *	\param	cache_ptr	Pointer to cache which contains the given buffer.
 *	\retval	SLAB_PTR	If Pointer to slab which contains the buffer
 *	\retval	NULL		If \a cache_ptr or \a buffer is NULL or if buffer is not found in tree.
*/
static SLAB_PTR SearchBufferInTree( VADDR buffer, CACHE_PTR cache_ptr )
{
	AVL_TREE_PTR root;
	VADDR start_va;
	SLAB_PTR slab_ptr;

 	/*! Without a valid buffer and cache_ptr we can't do anything */
	if (!cache_ptr || !buffer)
	{
		return NULL;
	}

	root = cache_ptr->in_use_slab_tree_root;
	while ( root )
	{
		slab_ptr = STRUCT_ADDRESS_FROM_MEMBER( root, SLAB, in_use_tree);
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
		else /*! if ( buffer > (start_va + cache_ptr->slab_metadata_offset) ) */
		{
			if ( IS_AVL_TREE_RIGHT_LIST_END(root) )
				return NULL;
			root = AVL_TREE_RIGHT_NODE(root);
		}
	}
	return NULL;
}

/*!
 * 	\fn		InitSlab
 *	\brief					Initializes the contents of a slab.
 *	\param	slab_ptr		Pointer to slab that has to be initialized.
 *	\param	buffer_count	Count of buffers in this slab. 
 *	\retval void 			No return value.
 */
static void InitSlab(SLAB_PTR slab_ptr, UINT32 buffer_count)
{
	int nbytes;
	/*! Iinitialize the tree and list */
	InitList( &(slab_ptr->partially_free_list) );
	InitAvlTreeNode( &(slab_ptr->in_use_tree), 0);
	InitList( &(slab_ptr->completely_free_list) );
	
	/*!	\todo	Call the constructor on each buffer */
	
	/*!	All buffers are free */
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
 * 	\fn		AllocateSlabToCache
 *	\brief					Allocates a PAGE from VM and Adds it to the cache
 *	\param	cache_ptr		Pointer to my cache entry.
 *	\param	immediate_use	An Integer, indicating if free buffer from the new slab is used immediately.
 *	\retval	0				If successfully fetched from VM 
 *	\retval	-1				If failure.
 *	\note 					Holds a lock to cache pointer.
 *	\todo					For performance the flag immediate use can be used as hint.
*/
static int AllocateSlabToCache(CACHE_PTR cache_ptr, int immediate_use)
{
	VADDR slab_start;
	
	/*! \todo Add conditon to check max_slabs count and exit */
#ifdef SLAB_STAT_ENABLED
	cache_ptr->stat.vm_alloc_calls++;
#endif
	slab_start = (VADDR) VM_ALLOC( cache_ptr->slab_size );
	if ( slab_start == NULL )
		return -1;

	if( AddSlabToCache(cache_ptr, slab_start) != NULL )
		return -1;
	
	return 0;
}

/*!
 * 	\fn		RemoveInUseTree
 *	\brief				Remove the entire in_use_tree 
 *	\param	cache_ptr	Pointer to cache which contains the tree to be removed.
 *	\retval	void		No return value.
*/
static void RemoveInUseTree(CACHE_PTR cache_ptr)
{
	while(cache_ptr->in_use_slab_tree_root)
	{
		AVL_TREE_PTR root = cache_ptr->in_use_slab_tree_root;
		SLAB_PTR slab_ptr;
		int i, count;

		RemoveNodeFromAvlTree( &(cache_ptr->in_use_slab_tree_root), root, 0, slab_inuse_tree_compare);
		slab_ptr = STRUCT_ADDRESS_FROM_MEMBER( root, SLAB, in_use_tree);
		cache_ptr->free_buffer_count -= (cache_ptr->slab_buffer_count - slab_ptr->used_buffer_count);
		/*clear the buffer usage bitmap */
		for(i=0, count=slab_ptr->used_buffer_count; count >=0; i++, count -= 8)
			slab_ptr->buffer_usage_bitmap[i] = 0;

		slab_ptr->used_buffer_count = 0;
		AddToCompletelyFreeList(cache_ptr, slab_ptr);
	}
}

/*!
 * 	\fn		GetFreeBufferFromCache
 *	\brief				Gets a free Buffer from the given slab.
 *	\param	cache_ptr	Pointer to cache which has the free buffer.
 *	\retval	VADDR		Virtual address of the free buffer.
 *	\note				Hold a lock to cache pointer.
*/
static VADDR GetFreeBufferFromCache(CACHE_PTR cache_ptr)
{
	SLAB_PTR slab_ptr;
	SLAB_STATE old_state, new_state;
	VADDR ret_va;
	UINT32 free_buffer_index = 0;
	
	slab_ptr = cache_ptr->partially_free_slab_list_head;
	/*!	\code	If partial free list is empty, get it from completely free list
	 *	if ( slab_ptr == NULL )
	 *		slab_ptr = cache_ptr->completely_free_slab_list_head;
	 *	\endcode
	 */
	if ( slab_ptr == NULL )
		slab_ptr = cache_ptr->completely_free_slab_list_head;
	
	/*! \code	Atleast one buffer should be free
	 *	assert( slab_ptr );
	 *	assert ( slab_ptr->used_buffer_count < cache_ptr->slab_buffer_count );
	 *	\endcode
	 */
	assert( slab_ptr );
	assert ( slab_ptr->used_buffer_count < cache_ptr->slab_buffer_count );
	
	/*! move the slab in/out of the differnt lists/tree */
	old_state = GetSlabState( cache_ptr, slab_ptr);
	slab_ptr->used_buffer_count++;
	new_state = GetSlabState( cache_ptr, slab_ptr);
	ManageSlabStateTransition( cache_ptr, slab_ptr, old_state, new_state );
	
	/*! \code	Find the first free buffer
	 *	if ( FindFirstClearBitInBitArray((void*)(slab_ptr->buffer_usage_bitmap), cache_ptr->slab_buffer_count, &free_buffer_index) == -1 )
	 *	\endcode
	 */
	if ( FindFirstClearBitInBitArray((void*)(slab_ptr->buffer_usage_bitmap), cache_ptr->slab_buffer_count, &free_buffer_index) == -1 )
		return NULL;
	
	/*! \code	Set the bitmap to indicate the buffer is used
	 *	SetBitInBitArray((void*)(slab_ptr->buffer_usage_bitmap), free_buffer_index );
	 * \endcode
	 */
	SetBitInBitArray((void*)(slab_ptr->buffer_usage_bitmap), free_buffer_index );
	
	cache_ptr->free_buffer_count --;
	
	/*! \code	Calculate the virtual address
	 *	ret_va = SLAB_START(slab_ptr, cache_ptr) + ( cache_ptr->buffer_size * free_buffer_index);
	 *	\endcode
	 */
	ret_va = SLAB_START(slab_ptr, cache_ptr) + ( cache_ptr->buffer_size * free_buffer_index);
	
	return ret_va;
}


/*!
 * 	\fn 	InitSlabAllocator
 *	\brief				Initializes a slab allocator. This is a 1 time operation.
 *	\param	page_size	Size of virtual page.
 *	\param	v_alloc		Function Pointer to virtual alloc.
 *	\param	v_free		Function pointer to virtual free.
 *	\param	v_protect	Function pointer to virtual protect.
 *	\return	0			On sucess
 *	\return	-1			On failure
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
 * 	\fn		InitCache
 *	\brief							Initializes an empty cache of specified buffer size.
 *	\param	new_cache				A static cache created in data segment.
 *	\param	size					Size of the buffers in cache.
 *	\param	free_slabs_threshold	Threshold to start VM operation.
 *	\param	min_buffers				Minimum no of buffers to be present always.
 *	\param	max_slabs				Maximum no of slabs allowed.
 *	\param	constructor				Function pointer to a function which initializes the newly created slab.
 *	\param	destructor				Function pointer to function which reuses a slab.
 *	\retval	0						If cache is created.(Success)
 *	\retval	-1						If cache is not created.(Failure)
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
	/*!	\code	recalcualte the buffer count
	 *	buf_count = (new_cache->slab_size - sizeof(SLAB) - bitmap_size) / new_cache->buffer_size;
	 *	\endcode
	 */
	buf_count = (new_cache->slab_size - sizeof(SLAB) - bitmap_size) / new_cache->buffer_size;
	
	new_cache->slab_metadata_size = sizeof(SLAB) + buf_count/BITS_PER_BYTE;
	/*!	\code	align the size
	 *	new_cache->slab_metadata_size = ALIGN_UP(new_cache->slab_metadata_size, 2);
	 *	\endcode
	 */
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
 *  \fn		DestroyCache
 *	\brief				Destroy the cache and return the vm_pages to VM subsystem. 
 *	\param	rem_cache	Pointer to cache which is to be destroyed.
 *	\retval	void		No return value
*/
void DestroyCache(CACHE_PTR rem_cache)
{
	UINT32 free_slabs;
	SLAB_PTR slab_ptr;
	VADDR rem_va;

	/*!	\code	Get a lock to cache
	 *	SpinLock( &(rem_cache->slock) );
	 *	\endcode
	 */
	SpinLock( &(rem_cache->slock) );

	/*! Move all partially used slabs to compeletely free slabs list */
	slab_ptr = rem_cache->partially_free_slab_list_head;
	while( slab_ptr != NULL )
	{
		SLAB_STATE old_state = GetSlabState( rem_cache, slab_ptr);
		
		rem_cache->free_buffer_count += slab_ptr->used_buffer_count;
		slab_ptr->used_buffer_count = 0;
		
		ManageSlabStateTransition( rem_cache, slab_ptr, old_state, GetSlabState(rem_cache, slab_ptr) );
		
		/*! get next partially used slab */
		slab_ptr = rem_cache->partially_free_slab_list_head;
	}

	/*!	\code Now remove all slabs from completely FULL slab list. This is possible by deleting all nodes from the in use tree.
	 *	RemoveInUseTree(rem_cache);
	 * 	\endcode
	 */
	RemoveInUseTree(rem_cache);
	
	/*!	\code	Before proceeding, make sure this cache is no more used by anybody.
	 *	assert( rem_cache->in_use_slab_tree_root == NULL);
	 *	\endcode
	 */
	assert( rem_cache->in_use_slab_tree_root == NULL);

	/*! Now all slabs are in completely free list. Free the vm_pages inside slabs pointed by completely free slab list and the slabs themselves. */
	free_slabs = rem_cache->free_slabs_count;
	while( free_slabs )
	{
		slab_ptr =  rem_cache->completely_free_slab_list_head;
		rem_cache->completely_free_slab_list_head = STRUCT_ADDRESS_FROM_MEMBER( rem_cache->completely_free_slab_list_head->completely_free_list.next, SLAB, completely_free_list);
		RemoveFromCompletelyFreeList( rem_cache, slab_ptr );
		
		/*! Now get the starting address of slab */
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
 *	\fn		AddSlabToCache
 *	\brief				Adds the given slab to cache.
 *	\param	cache_ptr	Pointer to my cache entry.
 *	\param	slab_start	Virtual address of slab starting address.
 *	\retval	0			If successfully fetched from VM.
 *	\retval	-1			If failure.
*/
int AddSlabToCache(CACHE_PTR cache_ptr, VADDR slab_start)
{
	SLAB_PTR slab_ptr;
	
	if ( slab_start == NULL )
		return -1;
	
	/*!	\code	Calculate the correct slab meta data and initialize it
	 *	slab_ptr = (SLAB_PTR) (slab_start + cache_ptr->slab_metadata_offset);
	 *	InitSlab(slab_ptr, cache_ptr->slab_buffer_count);
	 *	\endcode
	 */
	slab_ptr = (SLAB_PTR) (slab_start + cache_ptr->slab_metadata_offset);
	InitSlab(slab_ptr, cache_ptr->slab_buffer_count);
	
	ManageSlabStateTransition( cache_ptr, slab_ptr, SLAB_STATE_NEW, SLAB_STATE_FREE );
	
	return 0;
}

/*!
 * 	\fn		AllocateBuffer
 *	\brief				Gets a free buffer from cache. 
 *	\param	cache_ptr	Pointer to cache from which buffers are wanted.
 *	\param 	flag		To indicate if this function can sleep(0) or not(1).
 *	\retval	void*		On Success: Virtual address of a free buffer
 *	\retval	NULL		On Failure.
*/
void* AllocateBuffer(CACHE_PTR cache_ptr, UINT32 flag)
{
	VADDR ret_va = NULL;
	SpinLock(&(cache_ptr->slock));

#ifdef SLAB_STAT_ENABLED
	cache_ptr->stat.alloc_calls++;
#endif
	/*! If no free buffer is available, try to get it from free slab */
	if ( cache_ptr->free_buffer_count == 0 )
	{
		/*! if no free slab list get it from VM*/
		if ( cache_ptr->free_slabs_count == 0 )
		{
			/*! If allowed to sleep, try to get from VM or else return failure */
			if ( flag & CACHE_ALLOC_NO_SLEEP )
			{
				goto FINDING_BUFFER_DONE;
			}
		
			/*! Allocate a slab by calling VM */
			if ( AllocateSlabToCache(cache_ptr, TRUE) == -1 )
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
 * 	\fn		FreeBuffer
 *	\brief				Free A buffer in it's slab. If all buffers in the slab are free, move the slab to completely free slab list.
 *	\param	buffer		Pointer to buffer which is to be freed.
 *	\param	cache_ptr	Pointer to cache which contans the buffer.
 *	\retval	0			On Success:	If freed successfully.
 *	\retval	-1			On Failure: If given buffer isn't found in the cache.
 *	\note				Holds a lock to cache_ptr
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

	/*!	\code	Find the slab which contains this buffer, using in_use_slab_tree.
	 *	slab_ptr = SearchBufferInTree( (VADDR)(buffer), cache_ptr);
	 *	\endcode
	 */
	slab_ptr = SearchBufferInTree( (VADDR)(buffer), cache_ptr);
	if ( slab_ptr == NULL )
	{
		return -1;
	}

	/*!	\code	Clear the corresponding bit in buffer_usage_bitmap.
	 *	va_start = SLAB_START(slab_ptr, cache_ptr);
	 *	buffer_index = ( ((VADDR)buffer - va_start) / (cache_ptr->buffer_size) );
	 *	\endcode
	 */
	va_start = SLAB_START(slab_ptr, cache_ptr);
	buffer_index = ( ((VADDR)buffer - va_start) / (cache_ptr->buffer_size) );

	/*!	\code	See if this buffer is presently used.
	 *	byte = GetBitFromBitArray( (void*)(slab_ptr->buffer_usage_bitmap), buffer_index );
	 *	\endcode
	 */
	byte = GetBitFromBitArray( (void*)(slab_ptr->buffer_usage_bitmap), buffer_index );
	if(byte == 0)
	{
		return -1;
	}

	ClearBitInBitArray( (void*)(slab_ptr->buffer_usage_bitmap), buffer_index );	
	
	old_state = GetSlabState( cache_ptr, slab_ptr);
	slab_ptr->used_buffer_count --;
	cache_ptr->free_buffer_count ++;
	new_state = GetSlabState( cache_ptr, slab_ptr);
	
	ManageSlabStateTransition( cache_ptr, slab_ptr, old_state, new_state );
	
	/*!	\todo	If free buffer count is greater than free_slabs_threshold, then start VM operation */
	return 0;
}


/*!
 *	\fn		GetCacheStatistics
 *	\brief							Returns the cache statistics structure pointer.
 *	\param	cache_ptr				Pointer to cache for which stats are required.
 *	\retval	CACHE_STATISTICS_PTR	On SUCCESS: Returns the cache statistics structure pointer.
 *	\retval	NULL					On Failure: If stats are not enabled.
*/
CACHE_STATISTICS_PTR GetCacheStatistics(CACHE_PTR cache_ptr)
{
	CACHE_STATISTICS_PTR ret = NULL;
#ifdef SLAB_STAT_ENABLED
	ret = &cache_ptr->stat;
#endif
	return ret;
}
