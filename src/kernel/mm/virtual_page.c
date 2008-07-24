/*!
	\file		src/kernel/mm/virtual_page.c	
	\author		Samuel
	\version 	3.0
	\date	
  			Created: 02-Jun-2008 10:22pm
  			Last modified: 02-Jun-2008 10:22pm
	\brief	virtual page related routines
*/
#include <kernel/mm/vm.h>
#include <kernel/mm/virtual_page.h>
#include <kernel/mm/pmem.h>
#include <kernel/debug.h>
#include <string.h>

/*virtual page - avl tree*/
#define VP_AVL_TREE(vp)	(&(vp)->free_tree.avltree)

static void inline AddVirtualPageToVmFreeTree(VIRTUAL_PAGE_PTR vp);
static void InitVirtualPage(VIRTUAL_PAGE_PTR vp, UINT32 physical_address);
static VIRTUAL_PAGE_PTR FindFreeVirtualPageRange(UINT32 total_pages_required);

static void AddVirtualPageToActiveLRUList(VIRTUAL_PAGE_PTR vp);
static void AddVirtualPageToInactiveLRUList(VIRTUAL_PAGE_PTR vp);
static void RemoveVirtualPageFromLRUList(VIRTUAL_PAGE_PTR vp);
static COMPARISION_RESULT free_range_compare_fn(BINARY_TREE_PTR node1, BINARY_TREE_PTR node2);

/*! Creates and Initializes the virtual page array
	\param vpa	- starting address of the virtual page array
	\param page_count - total number of virtual pages
	\param start_physical_address - starting physical address of the first virtual page
*/
void InitVirtualPageArray(VIRTUAL_PAGE_PTR vpa, UINT32 page_count, UINT32 start_physical_address)
{
	int i;
	for(i=0; i<page_count ;i++)
	{
		InitVirtualPage( &vpa[i], start_physical_address );
		start_physical_address += PAGE_SIZE;
	}
	
}

/*! Initializes the virtual page
	\param vp - Address of the virtual page array
	\param physical_address - physical address managed/mapped by the virtual page
*/
static void InitVirtualPage(VIRTUAL_PAGE_PTR vp, UINT32 physical_address)
{
	memset(vp, 0, sizeof( VIRTUAL_PAGE ) );
	
	InitSpinLock( &vp->lock );
	InitList( &vp->lru_list );
	
	InitAvlTreeNode( VP_AVL_TREE(vp), 1 );
	
	vp->physical_address = physical_address;
	
	AddVirtualPageToVmFreeTree( vp );
}
/*! Returns the first virtual page of this free virtual page range.
	\param	vp - free virtual page 
	\return	First virtual page of the given virtual page
			NULL on failure
*/
inline VIRTUAL_PAGE_PTR GetFirstVirtualPage(VIRTUAL_PAGE_PTR vp)
{
	assert ( vp->free == 1 );
	
	/*if this page is not free then return null*/
	if ( vp->free == 0 )
		return NULL;
		
	/*if this page is not the first page*/
	if ( vp->free_first_page )
	{
		/*find the first page*/
		while ( vp->free_first_page )
		{
			vp = vp->free_first_page;
		}
		return vp;
	}
	
	/*this page is the first page*/
	return vp;
}

/*! Adds the given virtual page to the vm free pool
	\param vp - virtual page to add to the free pool
	\note vm_data and vp locks should be taken by the caller.
	
	The freed virtual page will be added to either the previous page's free range or 
	next page's free range. If previous page and next page are not in free state it is 
	added as new free range.
*/
static void inline AddVirtualPageToVmFreeTree(VIRTUAL_PAGE_PTR vp)
{
	VIRTUAL_PAGE_PTR p;
	
	assert( vp->free != 1 );
	
	/*mark this page as free*/
	vp->free = 1;
	vp->free_first_page = NULL;
	vp->free_size = 1;
			
	/*try to add to the previous free range*/
	p = PHYS_TO_VP( vp->physical_address - PAGE_SIZE );
	if ( p != NULL && p->free )
	{
		//get the first page
		p = GetFirstVirtualPage(p);
		assert( p != NULL );
		
		SpinLock(&vm_data.lock);
		
		//increase the size
		p->free_size++;
		//link to the first page
		vp->free_first_page = p;
		
		//need to balance the tree
		RemoveNodeFromAvlTree( &vm_data.free_tree, VP_AVL_TREE(p), 1, free_range_compare_fn );
		InsertNodeIntoAvlTree( &vm_data.free_tree, VP_AVL_TREE(p), 1, free_range_compare_fn );
		
		SpinUnlock(&vm_data.lock);
		return;
	}
	/*try to add to the next free range*/
	p = PHYS_TO_VP( vp->physical_address + PAGE_SIZE );
	if ( p != NULL && p->free )
	{
		SpinLock(&vm_data.lock);
		
		//current page becomes the first page
		vp->free_size = p->free_size + 1;
		p->free_first_page = vp;
		
		//remove the old first page
		RemoveNodeFromAvlTree( &vm_data.free_tree, VP_AVL_TREE(p), 1, free_range_compare_fn );
		//insert the current first page
		InsertNodeIntoAvlTree( &vm_data.free_tree, VP_AVL_TREE(vp), 1, free_range_compare_fn );

		SpinUnlock(&vm_data.lock);
		return;
	}
	/*add as a new free range*/		
	InsertNodeIntoAvlTree( &vm_data.free_tree, VP_AVL_TREE(vp), 1, free_range_compare_fn );
}

/*! Allocates a virtual page from the VM subsystem to the caller
	\param pages - number of contiguous pages requried
	\return on success returns pointer to the allocated virtual page 
		on failure returns NULL
		
	1) This routine gets the first virtual page of a free range by calling FindFreeVirtualPageRange()
	2) Removes the pages from the last of the range if the range is bigger than requested size.
*/
VIRTUAL_PAGE_PTR AllocateVirtualPages(int pages)
{
	VIRTUAL_PAGE_PTR vp, first_vp;
	int i;
	
	if ( vm_data.free_tree == NULL )
		return NULL;
		
	SpinLock( &vm_data.lock );
	first_vp = FindFreeVirtualPageRange(pages);
	
	/*if no range with requested size if found return NULL*/
	if ( first_vp == NULL )
	{
		SpinUnlock( &vm_data.lock );
		return NULL;
	}
	assert ( first_vp->free_size >= pages );
	
	vp = PHYS_TO_VP( first_vp->physical_address  + ((first_vp->free_size-1) * PAGE_SIZE) );
	assert ( vp != NULL );
	for(i=0; i<pages; i++ )
	{
		/*mark page as not free and add to LRU*/
		vp->free = 0;
		AddVirtualPageToActiveLRUList( vp );
		
		first_vp->free_size--;
		
		vp = PHYS_TO_VP( vp->physical_address - PAGE_SIZE );
	}
	/*remove the free range from the tree*/
	RemoveNodeFromAvlTree( &vm_data.free_tree, VP_AVL_TREE(first_vp), 1, free_range_compare_fn );
	/*if the free range is not fully used add it again to the free tree*/
	if ( first_vp->free_size )
		InsertNodeIntoAvlTree( &vm_data.free_tree, VP_AVL_TREE(first_vp), 1, free_range_compare_fn );
	
	SpinUnlock( &vm_data.lock );
	
	return vp;
}
/*! Adds the given virtual page to active lru list
	\param vp - virtual page to add
*/
static void AddVirtualPageToActiveLRUList(VIRTUAL_PAGE_PTR vp)
{
}
/*! Adds the given virtual page to inactive lru list
	\param vp - virtual page to add
*/
static void AddVirtualPageToInactiveLRUList(VIRTUAL_PAGE_PTR vp)
{
}
/*! Removes the given virtual page from lru list
	\param vp - virtual page to remove
*/
static void RemoveVirtualPageFromLRUList(VIRTUAL_PAGE_PTR vp)
{
}

/*! Frees one or more virtual page to the VM subsystem
	\param first_vp - starting virtual page in the range to free
	\param pages - total pages to free
*/
UINT32 FreeVirtualPages(VIRTUAL_PAGE_PTR first_vp, int pages)
{
	VIRTUAL_PAGE_PTR vp;
	int i;
	SpinLock( &vm_data.lock );
	vp = first_vp;
	for(i=0; i< pages; i++)
	{
		SpinLock( &vp->lock );
		
		if ( vp->free )
			panic("FreeVirtualPages() page is already free");
		
		RemoveVirtualPageFromLRUList( vp );
		AddVirtualPageToVmFreeTree( vp );
		
		vp = PHYS_TO_VP( vp->physical_address - PAGE_SIZE );
		
		SpinUnlock( &vp->lock );
		
	}
	SpinUnlock( &vm_data.lock );
	
	return 0;
}

/*! Finds the Virtual Page for a given physical address
	\param pysical_address - physical address for which virtual page to find
	
	\return NULL on failure
			virtual page ptr on success
*/
VIRTUAL_PAGE_PTR PhysicalToVirtualPage(UINT32 physical_address)
{
	int i,j;
	for(i=0; i<memory_area_count; i++ )
	{
		PHYSICAL_MEMORY_REGION_PTR pmr;
		for(j=0;j<memory_areas[i].physical_memory_regions_count;j++)
		{
			pmr = memory_areas[i].physical_memory_regions;
			if ( physical_address >= pmr->start_physical_address && physical_address < pmr->end_physical_address )
			{
				UINT32 index;
				index = (physical_address - pmr->start_physical_address)/PAGE_SIZE;
				assert ( index <= pmr->virtual_page_count);
				return &pmr->virtual_page_array[index];
			}
		}
	}
	return NULL;
}

/*! This function will find a free virtual page range with size greater than or equal to the requested size.
	\param total_pages_required - total pages required
	\return First virtual page in the free range
	\note 	1) This function does not allocate the pages.
			2) Caller is responsible for taking the lock.
			
	This function traverses the vm free avl tree to find a closest free range.
*/
static VIRTUAL_PAGE_PTR FindFreeVirtualPageRange(UINT32 total_pages_required)
{
	VIRTUAL_PAGE_PTR vp, result = NULL;
	
	vp = STRUCT_ADDRESS_FROM_MEMBER(vm_data.free_tree, VIRTUAL_PAGE, free_tree);
	while(1)
	{
		/*if exact size is found return immediately*/
		if ( vp->free_size == total_pages_required )
			return vp;
		/*more than required size is found - store the result and try to find a smaller one*/	
		else if ( vp->free_size > total_pages_required )
		{
			/*store the result*/
			if ( result == NULL || result->free_size > vp->free_size )
				result = vp;
			/*try to find smaller one*/
			if ( IS_AVL_TREE_LEFT_LIST_END( VP_AVL_TREE(vp) ) )
				break;/*smaller range is not exists so exit*/
			else
				vp = STRUCT_ADDRESS_FROM_MEMBER( AVL_TREE_LEFT_NODE( VP_AVL_TREE(vp) ), VIRTUAL_PAGE, free_tree);
		}
		else
		{
			/*try to find bigger range*/
			if ( IS_AVL_TREE_RIGHT_LIST_END( VP_AVL_TREE(vp) ) )
				break;/*large range is not exists so exit*/
			else
				vp = STRUCT_ADDRESS_FROM_MEMBER( AVL_TREE_RIGHT_NODE( VP_AVL_TREE(vp) ), VIRTUAL_PAGE, free_tree);
		}
	}
	return result;
}

/*! AVL Tree compare function for comparing two free virtual page ranges
*/
static COMPARISION_RESULT free_range_compare_fn(BINARY_TREE_PTR node1, BINARY_TREE_PTR node2)
{
	VIRTUAL_PAGE_PTR vp1, vp2;
	
	assert( node1 != NULL );
	assert( node2 != NULL );
	
	vp1 = STRUCT_ADDRESS_FROM_MEMBER(STRUCT_ADDRESS_FROM_MEMBER(node1, AVL_TREE, bintree), VIRTUAL_PAGE, free_tree);
	vp2 = STRUCT_ADDRESS_FROM_MEMBER(STRUCT_ADDRESS_FROM_MEMBER(node2, AVL_TREE, bintree), VIRTUAL_PAGE, free_tree);
	
	if ( vp1->free_size < vp2->free_size )
		return GREATER_THAN;
	else if ( vp1->free_size > vp2->free_size )
		return LESS_THAN;
	else 
		return EQUAL;
}
