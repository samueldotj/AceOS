/*!
	\file	kernel/mm/virtual_page.c	
	\brief	virtual page related routines
*/
#include <kernel/mm/vm.h>
#include <kernel/mm/virtual_page.h>
#include <kernel/mm/pmem.h>
#include <kernel/debug.h>
#include <string.h>

/*virtual page - avl tree*/
#define VP_AVL_TREE(vp)	(&(vp)->free_tree.avltree)

static int inline AddVirtualPageToVmFreeTree(VIRTUAL_PAGE_PTR vp, BOOLEAN check_sibling);
static void InitVirtualPage(VIRTUAL_PAGE_PTR vp, UINT32 physical_address);
static VIRTUAL_PAGE_PTR FindFreeVirtualPageRange(AVL_TREE_PTR free_tree, UINT32 total_pages_required);
static inline AVL_TREE_PTR * GetVirtualPageFreeTreeFromType(enum VIRTUAL_PAGE_RANGE_TYPE vp_range_type);
static inline AVL_TREE_PTR * GetVirtualPageFreeTreeFromPage(VIRTUAL_PAGE_PTR vp);
static int inline DownGradePhysicalRange(enum VIRTUAL_PAGE_RANGE_TYPE vp_requested_range_type, enum VIRTUAL_PAGE_RANGE_TYPE * current_vp_range_type);

static void AddVirtualPageToActiveLRUList(VIRTUAL_PAGE_PTR vp);
static void AddVirtualPageToInactiveLRUList(VIRTUAL_PAGE_PTR vp);
static void RemoveVirtualPageFromLRUList(VIRTUAL_PAGE_PTR vp);
static COMPARISION_RESULT free_range_compare_fn(BINARY_TREE_PTR node1, BINARY_TREE_PTR node2);

/*! Initializes a virtual page array
	\param vpa	- starting address of the virtual page array
	\param page_count - total number of virtual pages
	\param start_physical_address - starting physical address of the first virtual page
*/
void InitVirtualPageArray(VIRTUAL_PAGE_PTR vpa, UINT32 page_count, UINT32 start_physical_address)
{
	int i;
	AVL_TREE_PTR * vp_current_free_tree, * vp_prev_free_tree = NULL;
	for(i=0; i<page_count ;i++)
	{
		InitVirtualPage( &vpa[i], start_physical_address );
		start_physical_address += PAGE_SIZE;
	}
	/*Adding a page to Tree/list involves operations on other pages also, so do this after initializing a page*/
	for(i=0; i<page_count ;i++)
	{
		vp_current_free_tree = GetVirtualPageFreeTreeFromPage( &vpa[i] );
		AddVirtualPageToVmFreeTree( &vpa[i], vp_prev_free_tree == vp_current_free_tree);
		vp_prev_free_tree = vp_current_free_tree;
	}
}

/*! Initializes a virtual page
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
}
/*! Returns the first virtual page of this free virtual page range.
	\param	vp - free virtual page 
	\return	First virtual page of the given virtual page on success
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
	\param check_sibling - if true, check next and previous pages for merging
	\return 
		0 if the page is added to as new free range
		1 if the page is merged with previous page
		2 if the page is merged with next page
	\note vm_data and vp locks should be taken by the caller.
	
	The freed virtual page will be added to either the previous page's free range or 
	next page's free range. If previous page and next page are not in free state it is 
	added as new free range.
*/
static int inline AddVirtualPageToVmFreeTree(VIRTUAL_PAGE_PTR vp, BOOLEAN check_sibling)
{
	VIRTUAL_PAGE_PTR p;
	AVL_TREE_PTR * free_tree;
	assert( vp->free != 1 );
	
	free_tree = GetVirtualPageFreeTreeFromPage( vp );
	
	/*mark this page as free*/
	vp->free = 1;
	vp->free_first_page = NULL;
	vp->free_size = 1;
	
	if ( check_sibling )
	{
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
			RemoveNodeFromAvlTree( free_tree, VP_AVL_TREE(p), 1, free_range_compare_fn );
			InsertNodeIntoAvlTree( free_tree, VP_AVL_TREE(p), 1, free_range_compare_fn );
			
			SpinUnlock(&vm_data.lock);
			return 1;
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
			RemoveNodeFromAvlTree( free_tree, VP_AVL_TREE(p), 1, free_range_compare_fn );
			//insert the current first page
			InsertNodeIntoAvlTree( free_tree, VP_AVL_TREE(vp), 1, free_range_compare_fn );

			SpinUnlock(&vm_data.lock);
			return 2;
		}
	}
	/*add as a new free range*/		
	InsertNodeIntoAvlTree( free_tree, VP_AVL_TREE(vp), 1, free_range_compare_fn );
	return 0;
}

/*! returns virtual page free tree root for a given vp_range_type
	\param vp_range_type - virtual page range type
	\return pointer to the root of the free tree
*/
static inline AVL_TREE_PTR * GetVirtualPageFreeTreeFromType(enum VIRTUAL_PAGE_RANGE_TYPE vp_range_type)
{
	if ( vp_range_type == VIRTUAL_PAGE_RANGE_TYPE_NORMAL )
		return &vm_data.free_tree;
	else if ( vp_range_type == VIRTUAL_PAGE_RANGE_TYPE_BELOW_1MB )
		return &vm_data.free_tree_1M;
	else if ( vp_range_type == VIRTUAL_PAGE_RANGE_TYPE_BELOW_16MB )
		return &vm_data.free_tree_16M;
	else
		panic("Wrong VIRTUAL_PAGE_RANGE_TYPE");
		
	/*to satisfy compiler*/
	return NULL;
}
/*! returns virtual page free tree root for a given virtual page
	\param vp - virtual page
	\return pointer to the root of the free tree
*/
static inline AVL_TREE_PTR * GetVirtualPageFreeTreeFromPage(VIRTUAL_PAGE_PTR vp)
{
	UINT32 pa = VP_TO_PHYS(vp);
	if (  pa < (1024*1024) )
		return &vm_data.free_tree_1M;
	else if ( pa  < (1024*1024*16) )
		return &vm_data.free_tree_16M;
	else 
		return &vm_data.free_tree;
}

/*! selects the next virtual page range type
*/
static int inline DownGradePhysicalRange(enum VIRTUAL_PAGE_RANGE_TYPE vp_requested_range_type, enum VIRTUAL_PAGE_RANGE_TYPE * current_vp_range_type)
{
	if ( vp_requested_range_type == VIRTUAL_PAGE_RANGE_TYPE_NORMAL )
	{
		/*downgrade the vp range*/
		if ( *current_vp_range_type == VIRTUAL_PAGE_RANGE_TYPE_NORMAL )
			*current_vp_range_type = VIRTUAL_PAGE_RANGE_TYPE_BELOW_16MB;
		else if ( *current_vp_range_type == VIRTUAL_PAGE_RANGE_TYPE_BELOW_16MB )
			*current_vp_range_type = VIRTUAL_PAGE_RANGE_TYPE_BELOW_1MB;
		else
			return 0;
	}
	else if ( vp_requested_range_type == VIRTUAL_PAGE_RANGE_TYPE_BELOW_16MB )
	{
		if ( *current_vp_range_type == VIRTUAL_PAGE_RANGE_TYPE_BELOW_16MB )
			*current_vp_range_type = VIRTUAL_PAGE_RANGE_TYPE_BELOW_1MB;
		else
			return 0;
	}
	else
		return 0;
	return 1;
}
/*! Allocates a virtual page from the VM subsystem to the caller
	\param pages - number of contiguous pages requried
	\return on success returns pointer to the allocated virtual page 
		on failure returns NULL
		
	1) This routine gets the first virtual page of a free range by calling FindFreeVirtualPageRange()
	2) Removes the pages from the last of the range if the range is bigger than requested size.
*/
VIRTUAL_PAGE_PTR AllocateVirtualPages(int pages, enum VIRTUAL_PAGE_RANGE_TYPE vp_range_type)
{
	AVL_TREE_PTR * free_tree;
	VIRTUAL_PAGE_PTR vp, first_vp;
	enum VIRTUAL_PAGE_RANGE_TYPE current_vp_range_type = vp_range_type;
	int i;

try_different_vp_range:
	free_tree = GetVirtualPageFreeTreeFromType( current_vp_range_type );
	if ( *free_tree == NULL )
	{
		if ( DownGradePhysicalRange( vp_range_type, &current_vp_range_type ) )
			goto try_different_vp_range;
		else
			return NULL;
	}
		
	SpinLock( &vm_data.lock );
	first_vp = FindFreeVirtualPageRange(*free_tree, pages);
	
	/*if no range with requested size if found return NULL*/
	if ( first_vp == NULL )
	{
		SpinUnlock( &vm_data.lock );
		if ( DownGradePhysicalRange( vp_range_type, &current_vp_range_type ) )
			goto try_different_vp_range;
		else
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
	RemoveNodeFromAvlTree( free_tree, VP_AVL_TREE(first_vp), 1, free_range_compare_fn );
	/*if the free range is not fully used add it again to the free tree*/
	if ( first_vp->free_size )
		InsertNodeIntoAvlTree( free_tree, VP_AVL_TREE(first_vp), 1, free_range_compare_fn );
	
	SpinUnlock( &vm_data.lock );
	
	return vp;
}
/*! Adds the given virtual page to active lru list
	\param vp - virtual page to add
	\todo add implementation
*/
static void AddVirtualPageToActiveLRUList(VIRTUAL_PAGE_PTR vp)
{
}
/*! Adds the given virtual page to inactive lru list
	\param vp - virtual page to add
	\todo add implementation
*/
static void AddVirtualPageToInactiveLRUList(VIRTUAL_PAGE_PTR vp)
{
}
/*! Removes the given virtual page from lru list
	\param vp - virtual page to remove
	\todo add implementation
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
	AVL_TREE_PTR * free_tree;
	VIRTUAL_PAGE_PTR vp;
	int i;
	
	free_tree = GetVirtualPageFreeTreeFromPage( first_vp );
	
	SpinLock( &vm_data.lock );
	vp = first_vp;
	for(i=0; i< pages; i++)
	{
		SpinLock( &vp->lock );
		
		if ( vp->free )
			panic("FreeVirtualPages() page is already free");
		
		RemoveVirtualPageFromLRUList( vp );
		AddVirtualPageToVmFreeTree( vp, TRUE );
		
		vp = PHYS_TO_VP( vp->physical_address + PAGE_SIZE );
		
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
			pmr = &memory_areas[i].physical_memory_regions[j];
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
static VIRTUAL_PAGE_PTR FindFreeVirtualPageRange(AVL_TREE_PTR free_tree, UINT32 total_pages_required)
{
	VIRTUAL_PAGE_PTR vp, result = NULL;
	
	vp = STRUCT_ADDRESS_FROM_MEMBER(free_tree, VIRTUAL_PAGE, free_tree);
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
