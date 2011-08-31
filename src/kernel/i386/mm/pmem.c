/*!
	\file	kernel/i386/mm/pmem.c	
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

VM_UNIT_PTR kernel_pte_vm_unit=NULL;

/*! Checks whether the given VA is kernel VA or user VA*/
#define IS_KERNEL_ADDRESS(va)	(va >= KERNEL_VIRTUAL_ADDRESS_START)

MEMORY_AREA	memory_areas[MAX_MEMORY_AREAS];
int memory_area_count;

/*i386 arch specific kernel page directory*/
PAGE_DIRECTORY_ENTRY kernel_page_directory[PAGE_DIRECTORY_ENTRIES] __attribute__ ((aligned (PAGE_SIZE)));
PHYSICAL_MAP kernel_physical_map;

CACHE physical_map_cache;

static void CreatePageTable(PHYSICAL_MAP_PTR pmap, UINT32 va );

/*! Creates a new physical map and allocate page directory for it
	\param vmap - Virtual map for which physical map needs to be created
	\return on success physical map address 
			on failure null
*/
PHYSICAL_MAP_PTR CreatePhysicalMap(VIRTUAL_MAP_PTR vmap)
{
	PHYSICAL_MAP_PTR pmap;
	VADDR pte_va_start;
	UINT32 page_dir_pa;
	UINT32 pte_va_size;
	VM_UNIT_PTR user_pte_vm_unit;
	
	pmap = AllocateBuffer( &physical_map_cache, CACHE_ALLOC_SLEEP );
	if ( pmap == NULL )
		return NULL;
		
	pmap->virtual_map = vmap;
	/*allocate page directory from kernel map*/
	if ( AllocateVirtualMemory(&kernel_map, (VADDR*) &pmap->page_directory, 0, PAGE_SIZE, PROT_READ|PROT_WRITE, 0, NULL) != ERROR_SUCCESS )
	{
		FreeBuffer( pmap, &physical_map_cache );
		return NULL;
	}
	/*copy the kernel page directory*/
	memcpy(pmap->page_directory,  kernel_physical_map.page_directory, PAGE_SIZE );
	
	/*set the self mapping*/
	if ( TranslatePaFromVa( (VADDR )pmap->page_directory, &page_dir_pa ) == VA_NOT_EXISTS )
	{
		panic("pagedirectory is not in memory");
	}
	
	pmap->page_directory[PT_SELF_MAP_INDEX].all = (page_dir_pa | KERNEL_PTE_FLAG);
	
	/*Create vmdescriptor for user PageTable VAs*/
	pte_va_size = PAGE_SIZE * 754;
	pte_va_start = PT_SELF_MAP_ADDRESS;
	user_pte_vm_unit = CreateVmUnit(VM_UNIT_TYPE_PTE, VM_UNIT_FLAG_PRIVATE, pte_va_size);
	CreateVmDescriptor(vmap, pte_va_start, pte_va_start + pte_va_size, user_pte_vm_unit, &protection_kernel_write);
	
	/*Create vmdescriptor for user PageTable VAs*/
	pte_va_start = PT_SELF_MAP_ADDRESS + pte_va_start;
	pte_va_size = PAGE_SIZE * 260;
	CreateVmDescriptor(vmap, pte_va_start, pte_va_start + pte_va_size, kernel_pte_vm_unit, &protection_kernel_write);
		
	return pmap;
}

void MapKernelPageTableEntries()
{
	VM_DESCRIPTOR_PTR vd;
	VIRTUAL_PAGE_PTR vp;
	UINT32 size;
	int i;
	
	size = PAGE_SIZE * KB;
	/* map page tables */
	kernel_pte_vm_unit = CreateVmUnit(VM_UNIT_TYPE_PTE, VM_UNIT_FLAG_PRIVATE, size);
	vd = CreateVmDescriptor(&kernel_map, PT_SELF_MAP_ADDRESS, PT_SELF_MAP_ADDRESS + size, kernel_pte_vm_unit, &protection_kernel_write);
	assert(vd != NULL);
	
	/* Map all the PT pages created during boot time. */
	for(i=0;i<4;i++) {
		PAGE_DIRECTORY_ENTRY pde;
		
		pde = kernel_page_directory[PT_SELF_MAP_INDEX + i];
		if (pde.present) {
			vp = PhysicalToVirtualPage(PFN_TO_PA(pde.page_table_pfn));
			assert(vp != NULL);
			SetVmUnitPage(vd->unit, vp, i);
		}
	}
}

/*! Fills page table entry for a given VA. This function makes the corresponding VA to point to PA by filling PTEs.
	this function can be also called to change the protection.
	\param pmap - physical map 
	\param va - virtual address
	\param pa - physical address
	\param protection 
	
	\note physical map lock should be taken by the caller
*/
ERROR_CODE CreatePhysicalMapping(PHYSICAL_MAP_PTR pmap, UINT32 va, UINT32 pa, UINT32 protection)
{
	PAGE_DIRECTORY_ENTRY_PTR mapped_pde = NULL;
	PAGE_TABLE_ENTRY_PTR mapped_pte = NULL;
	PAGE_TABLE_ENTRY pte;
	UINT32 pfn = PA_TO_PFN(pa);
	
	assert( pmap != NULL );
	
	/* VA of PDE */
	mapped_pde = &pmap->page_directory[PAGE_DIRECTORY_ENTRY_INDEX(va)];
	/*create page table if not present*/
	if ( !mapped_pde->present )
	{
		CreatePageTable( pmap, va );
	}
	
	/* VA of PTE */
	mapped_pte = PT_SELF_MAP_PAGE_TABLE1_PTE(va);
	if ( mapped_pte->present )
	{
		/*if somebody else created this mapping return*/
		if ( mapped_pte->page_pfn == pfn )
		{
			/*! \todo - handle protection change here*/
			goto finish;
		}
		else
		{
			KPRINTF("VA %p PA %p Existing PA %p\n", va, pa, mapped_pte->page_pfn<<PAGE_SHIFT);
			panic("Trying to map over an existing map");
		}
	}
	else
	{
		//now mapping should present for the page table
		assert( mapped_pde->present );
		assert( protection != NULL );
		
		/* Set fields in PTE */
		pte.all = 0;
		pte.present = 1;
		pte.page_pfn = pfn;
		if ( protection & PROT_WRITE )
		{
			pte.write = 1;
		}
		if ( IS_KERNEL_ADDRESS(va) ) 
		{
			pte.global = 1;
		} else 
		{
			pte.user = 1;
		}
		
		/* Update the page table */
		mapped_pte->all = pte.all;
		asm volatile("invlpg (%%eax)" : : "a" (va));
	}

finish:
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
	VIRTUAL_PAGE_PTR vp;
	ERROR_CODE ret = ERROR_SUCCESS;
	
	size = PAGE_ALIGN_UP( size );
		
	SpinLock( &pmap->lock );
	
	for(i=0; i<size; i+=PAGE_SIZE )
	{
		vp = AllocateVirtualPages( 1, VIRTUAL_PAGE_RANGE_TYPE_NORMAL );
		if ( vp == NULL )
		{
			ret = ERROR_NOT_ENOUGH_MEMORY;
			break;
		}
			
		ret = CreatePhysicalMapping( pmap, va+i, VP_TO_PHYS(vp), protection );
		if ( ret != ERROR_SUCCESS )
			break;
		vp = PHYS_TO_VP( vp->physical_address + PAGE_SIZE );
	}
	
	if ( ret == ERROR_SUCCESS )
	{
		SpinUnlock( &pmap->lock );
		return ERROR_SUCCESS;
	}
		
	/*remove already entered mapping*/
	for(j=0; j<i; j+=PAGE_SIZE)
		RemovePhysicalMapping(pmap, va+i);
	/*\todo - free the allocated pages
	FreeVirtualPages( first_vp, NUMBER_OF_PAGES(size) );*/
	
	SpinUnlock( &pmap->lock );
	return ret;
}
/*! Removes the page table entries so that the virtual to physical mapping will be invalidated
	\param pmap - physical map from the which the mapping should be removed
	\param va - for which virtual address the mapping should be invalidated
	\return ERROR_CODE
	\note physical map's lock should be taken by the caller
*/
ERROR_CODE RemovePhysicalMapping(PHYSICAL_MAP_PTR pmap, UINT32 va)
{
	PAGE_DIRECTORY_ENTRY_PTR mapped_pde = NULL;
	PAGE_TABLE_ENTRY_PTR mapped_pte = NULL;
	
	mapped_pde = &pmap->page_directory[PAGE_DIRECTORY_ENTRY_INDEX(va)];
	mapped_pte = PT_SELF_MAP_PAGE_TABLE1_PTE(va);
	
	if ( !mapped_pde->present || !mapped_pte->present )
		return ERROR_SUCCESS;
	
	//clear the page table entry
	mapped_pte->all = 0;
	
	/*invalidate the TLB and cache*/
	InvalidateTlb( (void *)va );
	FlushCpuCache( TRUE );
		
	return ERROR_SUCCESS;
}

/*! creates page table for a given VA.
	\param pmap - physical map for which va mapping should be created
	\param va - virtual address
*/
static void CreatePageTable(PHYSICAL_MAP_PTR pmap, UINT32 va )
{
	int pd_index;
	PAGE_DIRECTORY_ENTRY_PTR page_dir;
	VADDR page_table_va;
	UINT32 pa;
	VIRTUAL_PAGE_PTR vp;
	VM_DESCRIPTOR_PTR vd;
	UINT32 vtop_index;
	
	assert(GetCurrentVirtualMap()->physical_map == pmap);
	assert(GetCurrentVirtualMap() == pmap->virtual_map);
	
	page_dir = pmap->page_directory;
	pd_index = PAGE_DIRECTORY_ENTRY_INDEX(va);
	
	/*if page table already present do nothing*/
	if ( page_dir[pd_index].present )
	{
		return;
	}

	/*allocate page table*/
	vp = AllocateVirtualPages(1, VIRTUAL_PAGE_RANGE_TYPE_NORMAL);
	assert ( vp != NULL );
	vd = GetVmDescriptor(pmap->virtual_map, va, 1);
	if ( vd )
	{
		vtop_index = ((va - vd->start) / PAGE_SIZE) + (vd->offset_in_unit/PAGE_SIZE);
		SetVmUnitPage(vd->unit, vp, vtop_index);
	}
	
	pa = vp->physical_address;
		
	/*enter pde*/
	page_dir[pd_index].all = pa | USER_PDE_FLAG;
	
	/*zero fill the page table*/
	page_table_va = PT_SELF_MAP_PAGE_TABLE1(va);
	assert( page_table_va != NULL );
	memset( (void *) page_table_va, 0, PAGE_SIZE);
	
	asm volatile("invlpg (%%eax)" : : "a" (page_table_va));
	
	/* \todo - Initiate IPI to other CPUs to start flush TLB */
}
/*! Reports the given virtual address range's status - readable/writeable or mapping not exists
	\param va - virtual address
	\param size - size of the va range
	\return 
			VA_WRITEABLE if the page is writeable
			VA_READABLE if the page is readable
			VA_NOT_EXISTS if there is no physical mapping for the given va
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
		if ( !pde->present )
			return VA_NOT_EXISTS;
		writable &= pde->write;
		
		pte = PT_SELF_MAP_PAGE_TABLE1_PTE(va);
		if ( !pte->present )
			return VA_NOT_EXISTS;
		writable &= pte->write;
		
		va += PAGE_SIZE;
	}
	if ( writable )
		return VA_WRITEABLE;
	return VA_READABLE;
}

/*! Returns Physical address for a given VA by looking inside the page tables
	\param va - IN  virtual address
	\param pa - OUT physical adress
	\return 
		VA_WRITEABLE if the page is writeable
		VA_READABLE if the page is readable
		VA_NOT_EXISTS if there is no physical mapping for the given va
*/
VA_STATUS TranslatePaFromVa(VADDR va, VADDR * pa)
{
	PAGE_DIRECTORY_ENTRY_PTR pde;
	PAGE_TABLE_ENTRY_PTR pte;
	
	pde = PT_SELF_MAP_PAGE_DIRECTORY_PTR(va);
	if ( !pde->present )
		return VA_NOT_EXISTS;
	pte = PT_SELF_MAP_PAGE_TABLE1_PTE(va);
	if ( !pte->present )
		return VA_NOT_EXISTS;
	
	if (pa)
	{
		*pa = PFN_TO_PA( pte->page_pfn );
	}
	if ( pte->write )
	{
		return VA_WRITEABLE;
	}
	return VA_READABLE;
}


/*! Internal function used to initialize the physical map structure*/
int PhysicalMapCacheConstructor( void *buffer)
{
	PHYSICAL_MAP_PTR physical_map = (PHYSICAL_MAP_PTR) buffer;
	memset(buffer, 0, sizeof(PHYSICAL_MAP) );
	
	InitSpinLock( &physical_map->lock );
	return 0;
}
/*! Internal function used to clear the physical map structure*/
int PhysicalMapCacheDestructor( void *buffer)
{
	PhysicalMapCacheConstructor(buffer);
	return 0;
}
