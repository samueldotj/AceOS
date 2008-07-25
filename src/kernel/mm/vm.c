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
VM_DESCRIPTOR kernel_static_code_descriptor;
VM_DESCRIPTOR kernel_static_data_descriptor;

VADDR kernel_free_virtual_address = NULL;

VM_PROTECTION protection_kernel_write = {0,0,1,1};
VM_PROTECTION protection_kernel_read = {0,0,0,1};
VM_PROTECTION protection_user_write = {1,1,0,0};
VM_PROTECTION protection_user_read = {0,1,0,0};

/*from kernel.ld*/
extern VADDR kernel_virtual_address, kernel_code_end, kernel_data_start, ebss;

static ERROR_CODE MapKernel();

/*! initializes the Virtual memory subsystem
*/
void InitVm()
{
	VADDR kmem_start_va, kmem_end_va;
	
	/*initialize the vm_data structure*/
	InitSpinLock(&vm_data.lock);
	vm_data.free_tree = NULL;
	
	vm_data.active_list = NULL;
	vm_data.inactive_list = NULL;
	
	vm_data.total_memory_pages = 0;
	vm_data.total_free_pages = 0;

	/*complete physical memory initialization*/
	InitPhysicalMemoryManagerPhaseII();
	kprintf("System memory: %dMB (PAGE_SIZE %d)\n", (vm_data.total_memory_pages * PAGE_SIZE) / (1024*1024), PAGE_SIZE );
	
	/*Initialize the kernel memory allocator*/
	kmem_start_va = kernel_free_virtual_address;
	InitKmem(kernel_free_virtual_address);
	kmem_end_va = kernel_free_virtual_address;
	
	/*map the kernel text, data*/
	MapKernel(kmem_start_va, kmem_end_va);
}

/*! maps the static code/data of the kernel
	and also reserved kmem
*/
static ERROR_CODE MapKernel(VADDR kmem_start_va, VADDR kmem_end_va)
{
	VM_UNIT_PTR vm_unit;
	/*map code and data*/
	InitVmDescriptor( &kernel_static_code_descriptor, &kernel_map, (VADDR)&kernel_virtual_address, (UINT32)&kernel_code_end, NULL, &protection_kernel_read);
	InitVmDescriptor( &kernel_static_data_descriptor, &kernel_map, (VADDR)&kernel_data_start, (UINT32)&ebss, NULL, &protection_kernel_write);
	
	/*map kmem*/
	vm_unit = CreateVmUnit( 0, kmem_end_va-kmem_start_va);
	CreateVmDescriptor(&kernel_map, kmem_start_va, kmem_end_va, vm_unit, &protection_kernel_write);
	/*todo update all the pages in the vtoparray*/
	
	return ERROR_SUCCESS;
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
	
	/*assume error*/
	* (va_ptr) = NULL;
	
	/*find a free vm range in the current virtual map*/
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

/*! Free the already allocated virtual memory range
*/
ERROR_CODE FreeVirtualMemory(VIRTUAL_MAP_PTR vmap, VADDR va, UINT32 size, UINT32 flags)
{
	return ERROR_SUCCESS;
}
