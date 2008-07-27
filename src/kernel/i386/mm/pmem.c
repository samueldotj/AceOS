/*!
	\file		src/kernel/i386/mm/pmem.c	
	\author		Dilip & Samuel
	\version 	3.0
	\date	
  			Created: 24-May-2008 14:37
  			Last modified: Tue May 27, 2008  11:44AM
	\brief	physical memory manager
*/
#include <ace.h>
#include <kernel/error.h>
#include <kernel/multiboot.h>
#include <kernel/debug.h>
#include <kernel/arch.h>
#include <kernel/mm/vm.h>
#include <kernel/mm/virtual_page.h>
#include <kernel/mm/pmem.h>
#include <kernel/i386/pmem.h>

#define IS_KERNEL_ADDRESS(va)	(va >= KERNEL_VIRTUAL_ADDRESS_START)

MEMORY_AREA	memory_areas[MAX_MEMORY_AREAS];
int memory_area_count;

/*i386 arch specific kernel page directory*/
PAGE_DIRECTORY_ENTRY kernel_page_directory[PAGE_DIRECTORY_ENTRIES] __attribute__ ((aligned (PAGE_SIZE)));
PHYSICAL_MAP kernel_physical_map;

static void CreatePageTable(PHYSICAL_MAP_PTR pmap, UINT32 va );


/*! Fills page table entry for a given VA. This function makes the corresponding VA to point to PA by filling PTEs.
	this function can be also called to change the protection.
	\param pmap - physical map 
	\param va - virtual address
	\param pa - physical address
	\param protection 
*/
ERROR_CODE CreatePhysicalMapping(PHYSICAL_MAP_PTR pmap, UINT32 va, UINT32 pa, UINT32 protection)
{
	PAGE_DIRECTORY_ENTRY_PTR mapped_pde = NULL;
	PAGE_TABLE_ENTRY_PTR mapped_pte = NULL;
	PAGE_TABLE_ENTRY pte;
	UINT32 pfn = PA_TO_PFN(pa);
	
	assert( pmap != NULL );
	
	mapped_pte = PT_SELF_MAP_PAGE_TABLE1_PTE(va);
	if ( IS_KERNEL_ADDRESS(va) )
		pte.all = KERNEL_PTE_FLAG;
	else
		pte.all = USER_PTE_FLAG;
		
	pte._.page_pfn = pfn;
	
	mapped_pde = &pmap->page_directory[PAGE_DIRECTORY_ENTRY_INDEX(va)];
	//create page table if not present
	if ( !mapped_pde->_.present )
	{
		CreatePageTable( pmap, va );
	}
	if ( mapped_pte->_.present && mapped_pte->_.page_pfn != pfn )
		panic("Trying to map over a existing map\n");
	
	if ( !mapped_pte->_.present )
	{
		//now mapping should present for the page table
		assert( mapped_pde->_.present );
		mapped_pte->all = pte.all;
	}
	else
	{
		//todo - handle protection change here
	}
	return ERROR_SUCCESS;
}

/*! Maps the given virtual address range by allocating physical addresses and entering page table entires
	\param pmap - Physical map on which the mapping should be placed
	\param va - Staring virtual addresss range
	\param size - size of the virtual adddress range
	\param protection - protection for this mapping.
*/
ERROR_CODE MapVirtualAddressRange(PHYSICAL_MAP_PTR pmap, UINT32 va, UINT32 size, UINT32 protection)
{
	UINT32 i,j;
	VIRTUAL_PAGE_PTR vp, first_vp;
	ERROR_CODE ret = ERROR_SUCCESS;
	
	size = PAGE_ALIGN_UP( size );
	vp = first_vp = AllocateVirtualPages( NUMBER_OF_PAGES(size) );
	if ( vp == NULL )
		return ERROR_NOT_ENOUGH_MEMORY;
	for(i=0; i<size; i+=PAGE_SIZE )
	{
		ret = CreatePhysicalMapping( pmap, va+i, VP_TO_PHYS(vp), protection );
		if ( ret != ERROR_SUCCESS )
		{
			/*remove already entered mapping*/
			for(j=0; j<i; j+=PAGE_SIZE)
				RemovePhysicalMapping(pmap, va+i);
			/*free the allocated pages*/
			FreeVirtualPages( first_vp, NUMBER_OF_PAGES(size) );
			return ret;
		}
		vp = PHYS_TO_VP( vp->physical_address + PAGE_SIZE );
	}
	return ret;
}
/*! Removes the page table entries so that the virtual to physical mapping will be invalidated
	\param pmap - physical map from the which the mapping should be removed
	\param va - for which virtual address the mapping should be invalidated
	\return ERROR_CODE
	\todo TLB invalidation
*/
ERROR_CODE RemovePhysicalMapping(PHYSICAL_MAP_PTR pmap, UINT32 va)
{
	PAGE_DIRECTORY_ENTRY_PTR mapped_pde = NULL;
	PAGE_TABLE_ENTRY_PTR mapped_pte = NULL;
	
	mapped_pde = &pmap->page_directory[PAGE_DIRECTORY_ENTRY_INDEX(va)];
	mapped_pte = PT_SELF_MAP_PAGE_TABLE1_PTE(va);
	
	//if we havent created a page table entry, just return success because nothing to remove
	if ( !mapped_pde->_.present || !mapped_pte->_.present )
		return ERROR_SUCCESS;
	
	//clear the page table entry
	mapped_pte->all = 0;
	
	/*invalidate the TLB and cache*/
	InvalidateTlb( (void *)va );
	FlushCpuCache( TRUE );
		
	return ERROR_SUCCESS;
}

/*! allocates a page for page table use*/
static UINT32 AllocatePageTable()
{
	VIRTUAL_PAGE_PTR vp;
	
	vp = AllocateVirtualPages(1);
	assert ( vp != NULL );
	return vp->physical_address;
}
/*! creates page table for a given VA.
	\param pmap - physical map on which the page table should be created
	\param va - virtual address for which page table needs to be created
*/
static void CreatePageTable(PHYSICAL_MAP_PTR pmap, UINT32 va )
{
	int pd_index;
	PAGE_DIRECTORY_ENTRY_PTR page_dir;
	
	page_dir = pmap->page_directory;
	pd_index = PAGE_DIRECTORY_ENTRY_INDEX(va);
	
	//if we dont have a page table, create it
	if ( !page_dir[ pd_index ]._.present )
	{
		/*allocate page table*/
		UINT32 pa = AllocatePageTable();
		
		/*enter pde*/
		page_dir[pd_index].all = pa | USER_PDE_FLAG;
	}
}
/*! Reports the given virtual address range's status
*/
VA_STATUS GetVirtualRangeStatus(VADDR va, UINT32 size)
{
	PAGE_DIRECTORY_ENTRY_PTR pde;
	PAGE_TABLE_ENTRY_PTR pte;
	VADDR end_va = va+size;
	int  writable=1;
	while( va < end_va )
	{
		pde = PT_SELF_MAP_PAGE_DIRECTORY_PTR(va);
		if ( !pde->_.present )
			return VA_NOT_EXISTS;
		writable &= pde->_.write;
		
		pte = PT_SELF_MAP_PAGE_TABLE1_PTE(va);
		if ( !pte->_.present )
			return VA_NOT_EXISTS;
		writable &= pte->_.write;
		
		va += PAGE_SIZE;
	}
	if ( writable )
		return VA_WRITEABLE;
	return VA_READABLE;
}
