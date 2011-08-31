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
#include <kernel/pm/thread.h>
#include <kernel/vfs/vfs.h>

VM_DATA vm_data;
VM_DESCRIPTOR kernel_static_code_descriptor;
VM_DESCRIPTOR kernel_static_data_descriptor;

VM_PROTECTION protection_kernel_write = {0,0,1,1};
VM_PROTECTION protection_kernel_read = {0,0,0,1};
VM_PROTECTION protection_user_write = {1,1,0,0};
VM_PROTECTION protection_user_read = {0,1,0,0};
VM_PROTECTION protection_all_write = {1,1,1,1};
VM_PROTECTION protection_all_read = {0,1,0,1};

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
	vm_data.free_tree_1M = NULL;
	vm_data.free_tree_16M = NULL;
	vm_data.active_list = NULL;
	vm_data.inactive_list = NULL;
	vm_data.total_memory_pages = 0;
	vm_data.total_free_pages = 0;

	/*complete physical memory initialization*/
	kernel_map.physical_map = &kernel_physical_map;
	InitPhysicalMemoryManagerPhaseII();
	kprintf("Total memory: %d KB (PAGE_SIZE %d)\n", (vm_data.total_memory_pages * PAGE_SIZE) / (1024), PAGE_SIZE );
	
	kernel_reserve_range.code_va_start = (VADDR)&kernel_virtual_address;
	kernel_reserve_range.code_va_end  = (VADDR)&kernel_code_end-1;
	/*\todo - get this from kernel symbol table*/
	kernel_reserve_range.code_pa_end =  kernel_reserve_range.code_pa_start + PAGE_ALIGN_UP( kernel_reserve_range.code_va_end - kernel_reserve_range.code_va_start )-1;
	
	kernel_reserve_range.data_va_start = (VADDR)&kernel_data_start;
	kernel_reserve_range.data_va_end = (VADDR)&ebss-1;
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
	
	kernel_reserve_range.kmem_va_end--;
	kernel_reserve_range.symbol_va_end--;
	kernel_reserve_range.module_va_end--;
	kernel_reserve_range.string_va_end--;
	
	/*Initialize the kernel memory allocator */
	InitKmem();

	/*map the kernel text, data - uses kmalloc() so do it after InitKmem()*/
	MapKernel();

	/*map kmem*/
	vm_unit = CreateVmUnit(VM_UNIT_TYPE_KERNEL, VM_UNIT_FLAG_PRIVATE, kernel_reserve_range.kmem_va_end - kernel_reserve_range.kmem_va_start);
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
	
	kernel_map.start = kernel_reserve_range.code_va_start;
	kernel_map.end = kernel_reserve_range.kmem_va_end;
	
	/*map kernel code*/
	vm_unit = CreateVmUnit(VM_UNIT_TYPE_KERNEL, VM_UNIT_FLAG_PRIVATE, kernel_reserve_range.code_va_end - kernel_reserve_range.code_va_start);
	InitVmDescriptor(&kernel_static_code_descriptor, &kernel_map, kernel_reserve_range.code_va_start, kernel_reserve_range.code_va_end, vm_unit, &protection_kernel_read);
	InitKernelDescriptorVtoP(&kernel_static_code_descriptor, kernel_reserve_range.code_va_start, kernel_reserve_range.code_va_end, kernel_reserve_range.code_pa_start);
	
	/*map kernel data*/
	vm_unit = CreateVmUnit(VM_UNIT_TYPE_KERNEL, VM_UNIT_FLAG_PRIVATE, kernel_reserve_range.data_va_end - kernel_reserve_range.data_va_start);
	InitVmDescriptor(&kernel_static_data_descriptor, &kernel_map, kernel_reserve_range.data_va_start, kernel_reserve_range.data_va_end, vm_unit, &protection_kernel_write);
	InitKernelDescriptorVtoP(&kernel_static_data_descriptor, kernel_reserve_range.data_va_start, kernel_reserve_range.data_va_end, kernel_reserve_range.data_pa_start);
	
	/*map modules*/
	vm_unit = CreateVmUnit(VM_UNIT_TYPE_KERNEL, VM_UNIT_FLAG_SHARED, kernel_reserve_range.module_va_end - kernel_reserve_range.module_va_start);
	vd = CreateVmDescriptor(&kernel_map, kernel_reserve_range.module_va_start, kernel_reserve_range.module_va_end, vm_unit, &protection_kernel_write);
	InitKernelDescriptorVtoP(vd, kernel_reserve_range.module_va_start, kernel_reserve_range.module_va_end, kernel_reserve_range.module_pa_start);

	/*map symbol table*/
	if ( kernel_reserve_range.symbol_pa_start )
	{
		vm_unit = CreateVmUnit(VM_UNIT_TYPE_KERNEL, VM_UNIT_FLAG_PRIVATE, kernel_reserve_range.symbol_va_end - kernel_reserve_range.symbol_va_start);
		vd = CreateVmDescriptor(&kernel_map, kernel_reserve_range.symbol_va_start, kernel_reserve_range.symbol_va_end, vm_unit, &protection_kernel_write);
		InitKernelDescriptorVtoP(vd, kernel_reserve_range.symbol_va_start, kernel_reserve_range.symbol_va_end, kernel_reserve_range.symbol_pa_start);
	}
	
	/*map string table*/
	if ( kernel_reserve_range.string_pa_start )
	{
		vm_unit = CreateVmUnit( VM_UNIT_TYPE_KERNEL, VM_UNIT_FLAG_PRIVATE, kernel_reserve_range.string_va_end - kernel_reserve_range.string_va_start );
		vd = CreateVmDescriptor(&kernel_map, kernel_reserve_range.string_va_start, kernel_reserve_range.string_va_end, vm_unit, &protection_kernel_write);
		InitKernelDescriptorVtoP(vd, kernel_reserve_range.string_va_start, kernel_reserve_range.string_va_end, kernel_reserve_range.string_pa_start );	
	}
	
	MapKernelPageTableEntries();

	return ERROR_SUCCESS;
}

/*! Initializes the vtop array for a given boot time kernel va and pa
*/
static void InitKernelDescriptorVtoP(VM_DESCRIPTOR_PTR vd, VADDR va_start, VADDR va_end, VADDR pa_start)
{
	VIRTUAL_PAGE_PTR vp;
	VADDR va, pa;
	int i;
	assert ( vd != NULL );
	for(i=0, va = va_start, pa = pa_start ; va < va_end;i++, pa += (VADDR)PAGE_SIZE, va += (VADDR)PAGE_SIZE )
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
	\param preferred_va - preferred starting virtual address 
	\param protection - protection for the mapping
	\return - 	Newly allocated VA on success
				NULL on failure
*/
VADDR MapPhysicalMemory(VIRTUAL_MAP_PTR vmap, UINT32 pa, UINT32 size, VADDR preferred_va, UINT32 protection)
{
	VADDR va;
	UINT32 i, vtop_index;
	VM_DESCRIPTOR_PTR vd;
	VIRTUAL_PAGE_PTR vp = NULL;

	assert(pa != 0);
	size = PAGE_ALIGN_UP(size);
	pa = PAGE_ALIGN(pa);
	if ( AllocateVirtualMemory( vmap, &va, preferred_va, size, protection, 0, NULL) != ERROR_SUCCESS )
	{
		return NULL;
	}
	vd = GetVmDescriptor(vmap, va, 1);
	assert ( vd != NULL  );
	assert( va >= vd->start && va <= vd->end );
	vtop_index = ((va - vd->start) / PAGE_SIZE) + (vd->offset_in_unit/PAGE_SIZE);
	assert( vtop_index <= (vd->unit->size/PAGE_SIZE) );
	
	for(i=0; i<size;i+=PAGE_SIZE )
	{
		if (GetCurrentVirtualMap() == vmap)
		{
			if ( CreatePhysicalMapping(vmap->physical_map, va+i, pa+i, protection) != ERROR_SUCCESS )
			{
				FreeVirtualMemory(vmap, va, size, 0);
				return NULL;
			}
		}
		else
		{
			/* if the PA is managed then create a lazy mapping */
			vp = PhysicalToVirtualPage(pa+i);
			if (vp == NULL)
			{
				panic("Request to map unmanaged page in different task map - Use kvm driver for that purpose");	
			}
			
			vd->unit->page_count++;
			vd->unit->vtop_array[vtop_index+(i/PAGE_SIZE)].vpage = (VIRTUAL_PAGE_PTR) ( ((VADDR)vp) | 1 );
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
	VADDR start, end;
	VM_PROTECTION_PTR prot;
	
	assert( size > 0 );
	/*page align the start and size*/
	end = PAGE_ALIGN_UP(preferred_start + size);
	preferred_start = PAGE_ALIGN(preferred_start);
	size = end - preferred_start;
	
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
	/*adjust the virtual map end 
	 * \todo - is this neccessary?*/
	if ( vmap->end < start + size )
		vmap->end = start + size;
	SpinUnlock(&vmap->lock);
	
	/*get the protection*/
	if ( vmap == &kernel_map )
	{
		if ( protection & PROT_WRITE )
			prot = &protection_kernel_write;
		else
			prot = &protection_kernel_read;
	}
	else
	{
		if ( protection & PROT_WRITE )
			prot = &protection_user_write;
		else
			prot = &protection_user_read;
	}
	
	/*create new vm_unit if unit is not provided*/
	if ( unit == NULL )
	{
		unit = CreateVmUnit(VM_UNIT_TYPE_ANONYMOUS, VM_UNIT_FLAG_SHARED, size);
	}
	/*link map, descriptor and unit*/
	CreateVmDescriptor(vmap, start, start+size-1, unit, prot);
	
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

/*! This function maps va to pa mapping in one virtual address space to another virtual address space
 * \param src_vmap - Source virtual map from which mapping should be copied.
 * \param src_va - Virtual address to be copied(better it is page aligned)
 * \param src_size - Total size to copy
 * \param dest_vmap - Destination virtual map to which mapping should be copied.
 * \param dest_preferred_va - preferred starting virtual address in the the destination map
 * \param dest_size - Size of the new allocation in destination map. It can be greater than src_size in that case, the remaining space will be zeroed. 
 * \param protection - protection for the new mapping
*/
ERROR_CODE CopyVirtualAddressRange(VIRTUAL_MAP_PTR src_vmap, VADDR src_va, UINT32 src_size, VIRTUAL_MAP_PTR dest_vmap, VADDR *dest_preferred_va, UINT32 dest_size, UINT32 protection)
{
	VM_DESCRIPTOR_PTR src_vd, dest_vd;
	VM_UNIT_PTR unit=NULL;
	VADDR va, unit_offset, va_difference;
	ERROR_CODE ret;
	VADDR org_src_size, org_dest_size, size;
	
	assert( src_size <= dest_size );
	
	/*preserve the passed sizes*/
	org_src_size = src_size;
	org_dest_size = dest_size;
	
	/*adjust starting and ending of the va range for page alignment*/
	va_difference = src_va - PAGE_ALIGN(src_va);
	src_va = PAGE_ALIGN(src_va);
	src_size = PAGE_ALIGN_UP(src_size+va_difference);
	dest_size = PAGE_ALIGN_UP(dest_size+va_difference);
	
	/*get vm descriptor of source*/
	src_vd = GetVmDescriptor(src_vmap, src_va, 1);
	if ( src_vd== NULL ) 
		return ERROR_NOT_FOUND;
	unit = src_vd->unit;
	unit_offset = src_vd->offset_in_unit + src_va - src_vd->start;
	assert( unit!=NULL );
	if ( unit_offset > unit->size || (unit->size - unit_offset) < src_size )
		return ERROR_INVALID_PARAMETER;
	
	/*allocate virtaul address range and map the same vm unit*/
	ret = AllocateVirtualMemory(dest_vmap, &va, *dest_preferred_va, src_size, protection, 0, unit);
	if ( ret != ERROR_SUCCESS )
		return ret;	
		
	/*set the offset in descriptor of the newly allocated va*/
	dest_vd = GetVmDescriptor(dest_vmap, va, src_size);
	assert( dest_vd != NULL );
	dest_vd->offset_in_unit = unit_offset;
	dest_vd->end = dest_vd->start + va_difference + org_src_size;

	/*do we need a zerofilling section after the mapping?*/
	if ( dest_size > src_size  )
	{
		VADDR second_va;
		size = dest_size - src_size;
		assert( size > 0 );
		/*we cant use the same unit as backing object - allocate virtaul address range with ZEROFILL unit*/
		ret = AllocateVirtualMemory(dest_vmap, &second_va, PAGE_ALIGN_UP(va+src_size), size, protection, 0, NULL);
		if ( ret != ERROR_SUCCESS )
		{
			FreeVirtualMemory(dest_vmap, va, src_size, 0);
			return ret;
		}
		/*\todo replace this assert with approriate return failure code*/
		assert( va+src_size == second_va );
	}
	
	/*although we cant protect the caller using the entire page, we should return where the actual data starts*/
	*dest_preferred_va = va + va_difference;
	return ERROR_SUCCESS;
}

/*! Returns the currrent virtual map
*/
inline VIRTUAL_MAP_PTR GetCurrentVirtualMap()
{
	THREAD_PTR thread;
	thread = GetCurrentThread();
	assert(thread != NULL);
	assert(thread->task != NULL);
	assert(thread->task->virtual_map != NULL);
	return thread->task->virtual_map;
}

/*! Maps a view of a file mapping into the address space of a calling process
	\param file_id - opened file number
	\param va - resulting mapped va
	\param protection - desired protection for mapped pages
	\param file_offset - file offset where the view begins.
	\param size - number of bytes to map
	\param preferred_start - preferred starting virtual address for the mapping
	\param flags - flags for mapping (shared/private etc)
*/
ERROR_CODE MapViewOfFile(int file_id, VADDR * va, UINT32 protection, UINT32 file_offset, UINT32 size, UINT32 preferred_start, UINT32 flags)
{
	VNODE_PTR vnode;
	VM_UNIT_PTR unit = NULL;
	VIRTUAL_MAP_PTR vmap;
	VM_DESCRIPTOR_PTR vm_descriptor;
	ERROR_CODE ret;
	
	assert( va );
	vnode = GetVnodeFromFile(file_id);
	/*if vnode not found it could be either because the file_id is wrong or some resource shortage*/
	if( vnode == NULL )
		return ERROR_INVALID_PARAMETER;
	
	if( file_offset!=0 && IS_PAGE_ALIGNED(file_offset) )
	if( !IS_PAGE_ALIGNED(size) )
		return ERROR_INVALID_PARAMETER;
	
	/*find a unit which map starts from the same offset
	\todo the following find algo should be modified to accomodate the following
		1) mapped unit offset is less than file_offset and mapped unit size is greater than size.
		2) the file is not shared - file open mode
	*/
	SpinLock(&vnode->lock);
	if ( vnode->unit_head != NULL )
	{
		if ( vnode->unit_head->offset <= file_offset && (vnode->unit_head->offset+vnode->unit_head->size)>= file_offset+size )
			unit = vnode->unit_head;
		else
		{
			LIST_PTR list;
			LIST_FOR_EACH(list, &vnode->unit_head->units_in_vnode_list)
			{
				VM_UNIT_PTR list_unit = STRUCT_ADDRESS_FROM_MEMBER(list, VM_UNIT, units_in_vnode_list );
				if ( list_unit->offset <= file_offset && (list_unit->offset+list_unit->size)>= file_offset+size )
				{
					unit = list_unit;
					break;
				}
			}
		}
	}
	SpinUnlock(&vnode->lock);
	/*if we dont have a mapped unit, create one*/
	if( unit == NULL)
	{
		unit = CreateVmUnit(VM_UNIT_TYPE_FILE_MAPPED, VM_UNIT_FLAG_SHARED, size);
		unit->offset = file_offset;
		unit->vnode = vnode;
		SpinLock(&vnode->lock);
		if ( vnode->unit_head == NULL )
			vnode->unit_head = unit;
		else
			AddToList( &vnode->unit_head->units_in_vnode_list, &unit->units_in_vnode_list );
		vnode->reference_count++;
		SpinUnlock(&vnode->lock);
	}
	
	assert ( unit->vnode  == vnode );

	/*create mapping*/
	vmap = GetCurrentVirtualMap();
	ret = AllocateVirtualMemory(vmap, va, preferred_start, size, protection, flags, unit);
	if( ret != ERROR_SUCCESS)
		return ret;
	
	/*update the vmdescriptor with correct offset*/
	vm_descriptor = GetVmDescriptor(vmap, *va, size);
	assert(vm_descriptor!=NULL);
	vm_descriptor->offset_in_unit = file_offset - unit->offset;;
	
	return ERROR_SUCCESS;
}

/*! Copy data from user space to kernel space(any user faults are handled and returned as failure)
 * \param user_va - user virtual address
 * \param kernel_va - kernel virtual address
 * \param length - size of the virtual address range to copy 
 * */
ERROR_CODE CopyFromUserSpace(void * user_va, void * kernel_va, size_t length)
{
	/*\todo - write proper code*/
	memmove( kernel_va, user_va, length );
	return ERROR_SUCCESS;
}

/*! Copy data from kernel to user space(any user faults are handled and returned as failure)
 * \param user_va - user virtual address
 * \param kernel_va - kernel virtual address
 * \param length - size of the virtual address range to copy 
 * */
ERROR_CODE CopyToUserSpace(void * user_va, void * kernel_va, size_t length)
{
	/*\todo - write proper code*/
	memmove( user_va, kernel_va, length );
	return ERROR_SUCCESS;
}

/*! Generic memory management fault handler
*/
ERROR_CODE MemoryFaultHandler(UINT32 va, int is_user_mode, int access_type)
{
	VIRTUAL_MAP_PTR virtual_map;
	VM_DESCRIPTOR_PTR vd;
	UINT32 vtop_index;
	VIRTUAL_PAGE_PTR vp = NULL;
	int zero_fill = FALSE;
	VADDR aligned_va;
	THREAD_PTR thread = GetCurrentThread();
	
	virtual_map = GetCurrentVirtualMap();
	if ( virtual_map == NULL )
	{
		kprintf("Virtual map is NULL for current thread %p\n", thread );
		return ERROR_NOT_FOUND;
	}

	aligned_va = PAGE_ALIGN(va);

retry:
	vd = GetVmDescriptor(virtual_map, aligned_va, 1);
	if ( vd == NULL  )
	{
		/*! if the faulting va is from userland kill the process*/
		if ( is_user_mode )
		{
			kprintf("User VA  %p not found - kill it\n", va);
			PrintVmDescriptors(virtual_map);
			/*\todo - kill the process*/
			return ERROR_NOT_FOUND;
		}
		/*user map wont have kmem descriptors - so try with kernel_map once before panicking*/
		if ( virtual_map != &kernel_map )
		{
			virtual_map = &kernel_map;
			kprintf("Kernel memory fault - va = %p virtual_map = %p, retrying..\n", va, virtual_map);
			goto retry;
		}
		kprintf("Kernel memory fault - va = %p virtual_map = %p\n", va, virtual_map);
		PrintVmDescriptors(virtual_map);
		/*! if the faulting va is kernel va and the kernel map doesnt have descriptor panic*/
		return ERROR_NOT_FOUND;	
	}

	//assert( va >= vd->start && va <= vd->end );
	
	vtop_index = ((va - vd->start) / PAGE_SIZE) + (vd->offset_in_unit/PAGE_SIZE);
	assert( vtop_index <= (vd->unit->size/PAGE_SIZE) );
	
	/*! if a page is already allocated use it else allocate new page*/
	if ( vd->unit->vtop_array[vtop_index].in_memory )
	{
		vp = (VIRTUAL_PAGE_PTR) ( (VADDR)vd->unit->vtop_array[vtop_index].vpage & ~1 );
	}
	else
	{
		/*if the page is backed by file, get it from file system*/
		if ( vd->unit->type == VM_UNIT_TYPE_FILE_MAPPED )
		{
			UINT32 file_offset;
			assert(vd->unit->vnode!=NULL);
			file_offset = vd->unit->offset + vd->offset_in_unit + PAGE_ALIGN(va-vd->start);
			assert( IS_PAGE_ALIGNED(file_offset) );
			vp = GetVnodePage(vd->unit->vnode, file_offset); 
			assert(vp!=NULL);
		}
		else
		{
			/*anonymous memory allocate memory and zero fill*/
			zero_fill = TRUE;
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
		}
		SetVmUnitPage(vd->unit, vp, vtop_index);
	}

	CreatePhysicalMapping(virtual_map->physical_map, va, vp->physical_address, vd->protection);
	
	if( zero_fill )
	{
		/*zero fill a anon page*/
		memset( (void *) aligned_va, 0, PAGE_SIZE);
	}
#if 0
	/* The following might look good but wont work - because files are loaded into a single page and shared by multiple descriptors(using a single vmunit)
	 * For example consider a 16K file is mapped into two descriptors 0-5k and 7-11K. In this case the file contents 4k-8k will be shared by descriptors
	 * so zeroing would cause problem.
	 */
	else if ( aligned_va+PAGE_SIZE > vd->end )
	{
		/*zero fill remaining in a page*/
		VADDR diff;
		diff = (aligned_va+PAGE_SIZE) - vd->end;
		memset( (void *)vd->end, 0, diff);
	}
#endif
	
	return ERROR_SUCCESS;
}
