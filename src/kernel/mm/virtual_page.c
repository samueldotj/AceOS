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

static void inline AddVirtualPageToVmFreeList(VIRTUAL_PAGE_PTR vp);
static void InitVirtualPage(VIRTUAL_PAGE_PTR vp, UINT32 physical_address);

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

static void InitVirtualPage(VIRTUAL_PAGE_PTR vp, UINT32 physical_address)
{
	memset(vp, 0, sizeof( VIRTUAL_PAGE ) );
	
	InitSpinLock( &vp->lock );
	InitList( &vp->page_list );
	InitList( &vp->lru_list );
	vp->physical_address = physical_address;
	
	AddVirtualPageToVmFreeList( vp );
}
/*! Adds the given virtual page to the vm free pool
	\param vp - virtual page to add to the free pool
	\note vm_data and vp locks should be taken by the caller.
*/
static void inline AddVirtualPageToVmFreeList(VIRTUAL_PAGE_PTR vp)
{
	assert( vp->free != 1 );
	vp->free = 1;
	if ( vm_data.free_page_head == NULL )
		vm_data.free_page_head = vp;
	else
		AddToList( &vm_data.free_page_head->page_list, &vp->page_list );
}

/*! Allocates a virtual page from the VM subsystem to the caller
	\return on success returns pointer to the allocated virtual page 
		on failure returns NULL
*/
VIRTUAL_PAGE_PTR AllocateVirtualPage()
{
	VIRTUAL_PAGE_PTR vp, next_vp;
	if ( vm_data.free_page_head == NULL )
		return NULL;
		
	SpinLock( &vm_data.lock );
	
	vp = vm_data.free_page_head;
	next_vp = STRUCT_FROM_MEMBER( VIRTUAL_PAGE_PTR, page_list, (vp)->page_list.next);
	RemoveFromList( &vp->page_list );
	if ( vp == next_vp )
		vm_data.free_page_head = NULL;
	else
		vm_data.free_page_head = next_vp;
	
	SpinUnlock( &vm_data.lock );
	
	return vp;
}

/*! give back a already allocated virtual page to the VM subsystem
	\param vp - virtual page to free
*/
UINT32 FreeVirtualPage(VIRTUAL_PAGE_PTR vp)
{
	SpinLock( &vm_data.lock );
	SpinLock( &vp->lock );
	AddVirtualPageToVmFreeList( vp );
	SpinUnlock( &vp->lock );
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
