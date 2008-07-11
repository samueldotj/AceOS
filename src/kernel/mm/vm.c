/*!
	\file		src/kernel/mm/vm.c	
	\author		Samuel
	\version 	3.0
	\date	
  			Created: 02-Jun-2008 10:34pm
  			Last modified: 02-Jun-2008 10:34pm
	\brief	vm related routines
*/
#include <string.h>
#include <ds/avl_tree.h>
#include <kernel/debug.h>
#include <kernel/mm/vm.h>
#include <kernel/mm/pmem.h>
#include <kernel/mm/kmem.h>

VM_DATA vm_data;

VM_PROTECTION protection_kernel_write = {0,0,1,1};
VM_PROTECTION protection_kernel_read = {0,0,1,0};
VM_PROTECTION protection_user_write = {1,1,0,0};
VM_PROTECTION protection_user_read = {1,0,0,0};

void InitVirtualMap(VIRTUAL_MAP_PTR vmap, PHYSICAL_MAP_PTR pmap);
void InitVmUnit(VM_UNIT_PTR unit, UINT32 type, UINT32 size);
/*! initializes the Virtual memory subsystem

*/
void InitVm()
{
	/*initialize the vm_data structure*/
	InitSpinLock(&vm_data.lock);
	vm_data.free_page_head = NULL;
	vm_data.inuse_page_head = NULL;

	/*complete physical memory initialization*/
	InitPhysicalMemoryManagerPhaseII();
}

/*! maps the static code/data of the kernel
	and also reserved kmem
*/
static ERROR_CODE MapKernel()
{
	
}

/*! Allocates virtual memory of given size 
	\param vmap   - Virtual Map from on which allocation is done
	\param va_ptr - Out parameter - returned va address is stored here
	\param preferred_start - preferred starting va
	\param size - required size ( in multiple of PAGE_SIZE)
	\param protection - page protection
	\param flags - private or shared or other flags..
	
	\return ERROR_SUCCESS  - sucess
			ERROR_NOT_ENOUGH_MEMORY - no free space on the map
*/
ERROR_CODE AllocateVirtualMemory(VIRTUAL_MAP_PTR vmap, VADDR * va_ptr, VADDR preferred_start, UINT32 size, UINT32 protection, UINT32 flags)
{
	VADDR start;
	VM_PROTECTION_PTR prot;
	VM_UNIT_PTR unit;
	
	* (va_ptr) = NULL;
	start = (VADDR)FindFreeVmRange(vmap, preferred_start, size, VA_RANGE_SEARCH_FROM_TOP);
	if ( start == NULL )
	{
		kprintf("AllocateVirtualMemory() - No memory range available\n");
		return ERROR_NOT_ENOUGH_MEMORY;
	}
	if ( vmap == &kernel_map )
		prot = &protection_kernel_write;
	else
	{
		if ( protection & PROT_WRITE )
			prot = &protection_user_write;
		else
			prot = &protection_user_read;
	}
	if ( flags )
	{
	}
	else
	{
		//private unit - so create new one
		unit = CreateVmUnit(0, size);
	}
	CreateVmDescriptor(vmap, start, start+size, unit, prot);
	
	* (va_ptr) = start;
	return ERROR_SUCCESS;
}
ERROR_CODE FreeVirtualMemory(VIRTUAL_MAP_PTR vmap, VADDR va, UINT32 size, UINT32 flags)
{
	return ERROR_SUCCESS;
}
