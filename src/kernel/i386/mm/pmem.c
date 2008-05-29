/*!
	\file		src/kernel/i386/mm/pmem.c	
	\author		Dilip & Samuel
	\version 	3.0
	\date	
  			Created: 24-May-2008 14:37
  			Last modified: Tue May 27, 2008  11:44AM
	\brief	physical memory manager
*/
#include <kernel/multiboot.h>
#include <kernel/mm/pmem.h>
#include <kernel/i386/pagetab.h>
#include <kernel/mm/vm.h>
#include <kernel/mm/virtual_page.h>
#include <kernel/debug.h>

MEMORY_REGION memory_region;

/*!
	\brief	 Initialize physical memory.

	\param	 mb: Pointer to multiboot info structure

	\return	 void
*/
void InitPhysicalMemory(MULTIBOOT_INFO_PTR mb_info)
{
	mb_info = (MULTIBOOT_INFO_PTR)BOOT_TO_KERNEL_ADDRESS(mb_info);

	MEMORY_MAP_PTR mmap = (MEMORY_MAP_PTR)( mb_info->mmap_addr );
	PHYSICAL_MEMORY_REGION_PTR pmem;
	int i=0, vpage_count;
	
	while(mmap < (MEMORY_MAP_PTR)(mb_info->mmap_addr) + mb_info->mmap_length)
	{
		pmem = &(memory_region.physical_memory_regions[i]);

		/* The higher order bits are 0 on a 32 bit machine */
		vpage_count = mmap->size / PAGE_SIZE;
		
		pmem->type = mmap->type;
		
		pmem->virtual_page_array = (VIRTUAL_PAGE_PTR)(mmap->base_addr_low);
		pmem->start_physical_address = PAGE_ALIGN_UP( mmap->base_addr_low + vpage_count*sizeof(struct virtual_page) );
		pmem->end_physical_address = mmap->base_addr_low + mmap->size;
		kprintf("type = %d start=%p end=%p\n", pmem->type, pmem->start_physical_address, pmem->end_physical_address); 
		
		mmap = (MEMORY_MAP_PTR)( (unsigned int)(mmap) + mmap->size + sizeof(mmap->size) );
		i++;
	}
}
