/*!
	\file		src/kernel/mm/vm.c	
	\brief	vm related routines
*/
#include <string.h>
#include <ds/avl_tree.h>
#include <kernel/debug.h>
#include <kernel/arch.h>
#include <kernel/mm/vm.h>
#include <kernel/mm/pmem.h>
#include <kernel/mm/kmem.h>
#include <kernel/mm/virtual_page.h>

VM_DATA vm_data;
VM_DESCRIPTOR kernel_static_code_descriptor;
VM_DESCRIPTOR kernel_static_data_descriptor;

VM_PROTECTION protection_kernel_write = {0,0,1,1};
VM_PROTECTION protection_kernel_read = {0,0,0,1};
VM_PROTECTION protection_user_write = {1,1,0,0};
VM_PROTECTION protection_user_read = {0,1,0,0};

KERNEL_RESERVE_RANGE kernel_reserve_range;

/*from kernel.ld*/
extern VADDR kernel_virtual_address, kernel_code_end, kernel_data_start, ebss;

static ERROR_CODE MapKernel();
static void InitKernelDescriptorVtoP(VM_DESCRIPTOR_PTR vd, VADDR va_start, VADDR va_end, VADDR pa_start);

/*! initializes the Virtual memory subsystem
*/
void InitVm()
{
	VIRTUAL_PAGE_PTR vp;
	VM_UNIT_PTR vm_unit;
	
	/*initialize the vm_data structure*/
	InitSpinLock(&vm_data.lock);
	vm_data.free_tree = NULL;
	vm_data.active_list = NULL;
	vm_data.inactive_list = NULL;
	vm_data.total_memory_pages = 0;
	vm_data.total_free_pages = 0;

	/*complete physical memory initialization*/
	InitPhysicalMemoryManagerPhaseII();
	kprintf("Total memory: %d KB (PAGE_SIZE %d)\n", (vm_data.total_memory_pages * PAGE_SIZE) / (1024), PAGE_SIZE );
	
	kernel_reserve_range.code_va_start = (VADDR)&kernel_virtual_address;
	kernel_reserve_range.code_va_end  = (VADDR)&kernel_code_end;
	/*\todo - get this from kernel symbol table*/
	kernel_reserve_range.code_pa_end =  kernel_reserve_range.code_pa_start + PAGE_ALIGN_UP( kernel_reserve_range.code_va_end - kernel_reserve_range.code_va_start );
	
	kernel_reserve_range.data_va_start = (VADDR)&kernel_data_start;
	kernel_reserve_range.data_va_end = (VADDR)&ebss;
	/*\todo - get this from kernel symbol table*/
	kernel_reserve_range.data_pa_start =  kernel_reserve_range.code_pa_end;

	/*lock the pages used by kernel code, 
	this needs to be done before initializing kmem because kmem might allocate virtual pages
	*/
	vp = PhysicalToVirtualPage( kernel_reserve_range.code_pa_start );
	assert( vp != NULL );
	ReserveVirtualPages( vp, (kernel_reserve_range.code_va_end-kernel_reserve_range.code_va_start)/PAGE_SIZE ); 
	/*lock the pages used by kernel data*/ 
	vp = PhysicalToVirtualPage( kernel_reserve_range.data_pa_start );
	assert( vp != NULL );
	ReserveVirtualPages( vp, (kernel_reserve_range.data_va_end-kernel_reserve_range.data_va_start)/PAGE_SIZE ); 
	
	/*Initialize the kernel memory allocator */
	InitKmem();
	
	/*map the kernel text, data - uses kmalloc() so do it after InitKmem()*/
	MapKernel();

	/*map kmem*/
	vm_unit = CreateVmUnit( 0, kernel_reserve_range.kmem_va_end - kernel_reserve_range.kmem_va_start);
	CreateVmDescriptor(&kernel_map, kernel_reserve_range.kmem_va_start, kernel_reserve_range.kmem_va_end, vm_unit, &protection_kernel_write);
}

/*! Updates the kernel virtual map data structures.
		1) Maps static kernel code 
		2) Maps static kernel data 
		3) Maps kmem
*/
static ERROR_CODE MapKernel()
{
	VM_UNIT_PTR vm_unit;
	VM_DESCRIPTOR_PTR vd;
	
	kernel_map.physical_map = &kernel_physical_map;
	kernel_map.start = kernel_reserve_range.code_va_start;
	kernel_map.end = kernel_reserve_range.kmem_va_end;
	
	/*map kernel code*/
	vm_unit = CreateVmUnit( 0, kernel_reserve_range.code_va_end - kernel_reserve_range.code_va_start);
	InitVmDescriptor( &kernel_static_code_descriptor, &kernel_map, kernel_reserve_range.code_va_start, kernel_reserve_range.code_va_end, vm_unit, &protection_kernel_read);
	InitKernelDescriptorVtoP(&kernel_static_code_descriptor, kernel_reserve_range.code_va_start, kernel_reserve_range.code_va_end, kernel_reserve_range.code_pa_start);
	
	/*map kernel data*/
	vm_unit = CreateVmUnit( 0, kernel_reserve_range.data_va_end - kernel_reserve_range.data_va_start);
	InitVmDescriptor( &kernel_static_data_descriptor, &kernel_map, kernel_reserve_range.data_va_start, kernel_reserve_range.data_va_end, vm_unit, &protection_kernel_write);
	InitKernelDescriptorVtoP(&kernel_static_data_descriptor, kernel_reserve_range.data_va_start, kernel_reserve_range.data_va_end, kernel_reserve_range.data_pa_start);
	
	/*map modules*/
	vm_unit = CreateVmUnit( 0, kernel_reserve_range.module_va_end - kernel_reserve_range.module_va_start );
	vd = CreateVmDescriptor(&kernel_map, kernel_reserve_range.module_va_start, kernel_reserve_range.module_va_end, vm_unit, &protection_kernel_write);
	InitKernelDescriptorVtoP(vd, kernel_reserve_range.module_va_start, kernel_reserve_range.module_va_end, kernel_reserve_range.module_pa_start );

	/*map symbol table*/
	if ( kernel_reserve_range.symbol_pa_start )
	{
		vm_unit = CreateVmUnit( 0, kernel_reserve_range.symbol_va_end - kernel_reserve_range.symbol_va_start );
		vd = CreateVmDescriptor(&kernel_map, kernel_reserve_range.symbol_va_start, kernel_reserve_range.symbol_va_end, vm_unit, &protection_kernel_write);
		InitKernelDescriptorVtoP(vd, kernel_reserve_range.symbol_va_start, kernel_reserve_range.symbol_va_end, kernel_reserve_range.symbol_pa_start );
	}
	
	/*map string table*/
	if ( kernel_reserve_range.string_pa_start )
	{
		vm_unit = CreateVmUnit( 0, kernel_reserve_range.string_va_end - kernel_reserve_range.string_va_start );
		vd = CreateVmDescriptor(&kernel_map, kernel_reserve_range.string_va_start, kernel_reserve_range.string_va_end, vm_unit, &protection_kernel_write);
		InitKernelDescriptorVtoP(vd, kernel_reserve_range.string_va_start, kernel_reserve_range.string_va_end, kernel_reserve_range.string_pa_start );	
	}

	/*todo update all the pages in the vtoparray*/
	
	return ERROR_SUCCESS;
}
static void InitKernelDescriptorVtoP(VM_DESCRIPTOR_PTR vd, VADDR va_start, VADDR va_end, VADDR pa_start)
{
	VIRTUAL_PAGE_PTR vp;
	VADDR va, pa;
	int i;
	assert ( vd != NULL );
	for(i=0, va = va_start, pa = pa_start ; va <= va_end;i++, pa += (VADDR)PAGE_SIZE, va += (VADDR)PAGE_SIZE )
	{
		vp = PhysicalToVirtualPage( pa );
		assert( vp != NULL );
		vd->unit->vtop_array[i].vpage = (VIRTUAL_PAGE_PTR)((VADDR)vp | 1);
	}
}

/*! Creates Virtual Mapping for a given physical address range
	\param vmap - virtual map 
	\param pa - physical address
	\param size - size of the physical range
	
		NULL on failure
*/
VADDR MapPhysicalMemory(VIRTUAL_MAP_PTR vmap, UINT32 pa, UINT32 size)
{
	VADDR va;
	UINT32 i;
	
	size = PAGE_ALIGN_UP(size);
	pa = PAGE_ALIGN(pa);
	if ( AllocateVirtualMemory( vmap, &va, 0, size, 0, 0, NULL) != ERROR_SUCCESS )
		return NULL;
	for(i=0; i<size;i+=PAGE_SIZE )
	{
		if ( CreatePhysicalMapping(vmap->physical_map, va+i, pa+i, 0) != ERROR_SUCCESS )
		{
			FreeVirtualMemory(vmap, va, size, 0);
			return NULL;
		}
	}
	return va;
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
ERROR_CODE AllocateVirtualMemory(VIRTUAL_MAP_PTR vmap, VADDR * va_ptr, VADDR preferred_start, UINT32 size, UINT32 protection, UINT32 flags, VM_UNIT_PTR unit)
{
	VADDR start;
	VM_PROTECTION_PTR prot;
	
	assert( size > 0 );
	size = PAGE_ALIGN_UP(size);
	
	/*assume error*/
	* (va_ptr) = NULL;
	
	SpinLock(&vmap->lock);
	/*find a free vm range in the current virtual map*/
	start = (VADDR)FindFreeVmRange(vmap, preferred_start, size, VA_RANGE_SEARCH_FROM_TOP);
	if ( start == NULL )
	{
		kprintf("AllocateVirtualMemory(%d) - No memory range available \n", size);
		SpinUnlock(&vmap->lock);
		return ERROR_NOT_ENOUGH_MEMORY;
	}
	if ( vmap->end < start + size )
		vmap->end = start + size;
	SpinUnlock(&vmap->lock);
		
	if ( vmap == &kernel_map )
		prot = &protection_kernel_write;
	else
	{
		if ( protection & PROT_WRITE )
			prot = &protection_user_write;
		else
			prot = &protection_user_read;
	}
	
	/*create new vm_unit(if private mapping)*/
	if ( !unit )
	{
		unit = CreateVmUnit(0, size);
	}
	/*link map, descriptor and unit*/
	CreateVmDescriptor(vmap, start, start+size, unit, prot);
	
	* (va_ptr) = start;
	return ERROR_SUCCESS;
}

/*! Free the already allocated virtual memory range
*/
ERROR_CODE FreeVirtualMemory(VIRTUAL_MAP_PTR vmap, VADDR va, UINT32 size, UINT32 flags)
{
	ERROR_CODE ret;
	UINT32 rem_va;
	
	/*remove the page table entries*/
	for(rem_va = va; rem_va < (va+size); rem_va+=PAGE_SIZE)
	{
		ret = RemovePhysicalMapping(vmap->physical_map, rem_va);
		if ( ret != ERROR_SUCCESS )
			return ret;
	}
	
	/*TODO - remove the vm descriptor entry*/
	return ERROR_SUCCESS;
}

/*! This function maps va to pa mapping in one virtual address space to another virtual address space*/
ERROR_CODE CopyVirtualAddressRange(VIRTUAL_MAP_PTR src_vmap, VADDR src_va, VIRTUAL_MAP_PTR dest_vmap, VADDR *dest_preferred_va, UINT32 dest_size, UINT32 protection)
{
	VM_DESCRIPTOR_PTR vd;
	VM_UNIT_PTR unit;
	VADDR va, unit_offset;
	ERROR_CODE ret;
	
	src_va = PAGE_ALIGN(src_va);
	dest_size = PAGE_ALIGN_UP(dest_size);
	
	vd = GetVmDescriptor(src_vmap, src_va);
	assert( vd != NULL );
	unit_offset = vd->offset_in_unit + PAGE_ALIGN(src_va - vd->start);
	unit = vd->unit;
	assert( unit!=NULL && unit_offset < unit->size );
	
	/*! allocate virtaul address range and map the same vm unit*/
	ret = AllocateVirtualMemory(dest_vmap, &va, *dest_preferred_va, dest_size, protection, 0, unit);
	if ( ret != ERROR_SUCCESS )
		return ret;
	
	*dest_preferred_va = va;
	/*if the descriptor maps only part of the vm_unit then update the starting offset in unit*/
	if ( unit_offset )
	{
		vd = GetVmDescriptor(dest_vmap, va);
		assert( vd != NULL );
		vd->offset_in_unit = unit_offset;
	}
	
	return ERROR_SUCCESS;
}

/*! Returns the currrent virtual map
	\todo Add code to get the current from current thread
*/
VIRTUAL_MAP_PTR GetCurrentVirtualMap()
{
	return &kernel_map;
}

/*! Generic memory management fault handler
*/
ERROR_CODE MemoryFaultHandler(UINT32 va, int is_user_mode, int access_type)
{
	VIRTUAL_MAP_PTR virtual_map;
	VM_DESCRIPTOR_PTR vd;
	UINT32 vtop_index;
	UINT32 protection = access_type;
	VIRTUAL_PAGE_PTR vp = NULL;
	
	/*! \todo implement user mode fault handling*/
	if ( is_user_mode )
		return ERROR_NOT_SUPPORTED;
		
	virtual_map = GetCurrentVirtualMap();
	assert( virtual_map != NULL );
	
	vd = GetVmDescriptor(virtual_map, va);
	/*! if the faulting va is kernel va and the kernel map doesnt have descriptor panic*/
	if ( vd == NULL && !is_user_mode )
	{
		kprintf("Kernel memory fault - va = %p virtual_map = %p\n", va, virtual_map);
		return ERROR_NOT_FOUND;	
	}
	
	vtop_index = ((va - vd->start) / PAGE_SIZE) + (vd->offset_in_unit/PAGE_SIZE);

	/*! if a page is already allocated use it else allocate new page*/
	if ( vd->unit->vtop_array[vtop_index].in_memory )
		vp = (VIRTUAL_PAGE_PTR) ( (VADDR)vd->unit->vtop_array[vtop_index].vpage & ~1 );
	else
	{
		vp = AllocateVirtualPages(1, VIRTUAL_PAGE_RANGE_TYPE_NORMAL);
		if ( vp == NULL )
		{
			kprintf("Unable to allocate PAGE during page fault\n");
			if ( is_user_mode )
			{
				/*! \todo make sure the process sleeps for some time*/
				return ERROR_RETRY;
			}
			else
				panic("Kernel resource shortage");
		}
		vd->unit->page_count++;
		vd->unit->vtop_array[vtop_index].vpage = (VIRTUAL_PAGE_PTR) ( ((VADDR)vp) | 1 );
	}
	CreatePhysicalMapping( virtual_map->physical_map, va, vp->physical_address, protection);
	
	return ERROR_SUCCESS;
}
