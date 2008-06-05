/*!
	\file		src/kernel/i386/mm/pmem_pa.c	
	\author		Samuel
	\version 	3.0
	\date	
  			Created: 01-Jun-2008 17:00
  			Last modified: 01-Jun-2008 17:00
	\brief	physical memory manager initialization routines
	\note		all these routines uses physical memory and dont know about va.
*/
#include <string.h>
#include <kernel/multiboot.h>
#include <kernel/debug.h>
#include <kernel/mm/vm.h>
#include <kernel/mm/virtual_page.h>
#include <kernel/mm/pmem.h>
#include <kernel/i386/pmem.h>

extern UINT32 ebss;

#define BOOT_ADDRESS(addr)	(((UINT32)addr - KERNEL_VIRTUAL_ADDRESS_TEXT_START)+KERNEL_PHYSICAL_ADDRESS_LOAD )

#define KERNEL_PTE_FLAG		(PAGE_PRESENT | PAGE_READ_WRITE | PAGE_SUPERUSER | PAGE_GLOBAL)

static UINT32 InitMemoryArea(MEMORY_AREA_PTR ma_pa, MEMORY_MAP_PTR memory_map_array, int memory_map_count);
static void * GetFreePhysicalPage();
static void InitKernelPageDirectory(UINT32 k_map_end);
static void EnterKernelPageTableEntry(UINT32 va, UINT32 pa);


/*!
	\brief	 Initialize physical memory.
	\param	 mb_info: Pointer to multiboot info structure
	\return	 void
*/
void InitPhysicalMemoryManagerPhaseI(unsigned long magic, MULTIBOOT_INFO_PTR mbi)
{
	UINT32 vpa_size;
	if ( magic != MULTIBOOT_BOOTLOADER_MAGIC )
		return;/*we will panic in main()*/
		
	//get physical address of the memory area - currently no NUMA support for i386
	* ((int *)BOOT_ADDRESS ( &memory_area_count ) )  = 1;
	vpa_size = InitMemoryArea( (MEMORY_AREA_PTR) BOOT_ADDRESS(memory_areas), (MEMORY_MAP_PTR)mbi->mmap_addr,  mbi->mmap_length / sizeof(MEMORY_MAP) );
	
	InitKernelPageDirectory( PAGE_ALIGN_UP(&ebss) );
}

/*! Intializes the memory area information
	\return size of all virtual page array in bytes
*/
static UINT32 InitMemoryArea(MEMORY_AREA_PTR ma_pa, MEMORY_MAP_PTR memory_map_array, int memory_map_count)
{
	UINT32 total_size = 0;	//total bytes occupied by virtual page array
	int i;
	for(i=0; i<memory_map_count; i++)
	{
		//use only available RAM
		if( memory_map_array[i].type == 1 )
		{
			PHYSICAL_MEMORY_REGION_PTR pmr_pa;
			int total_virtual_pages, virtual_page_array_size;

			//if memory start is less than 1MB then skip it because we cant use it
			if ( memory_map_array[i].base_addr_low < (1024 * 1024) )
				continue;
			//we cant use the kernel code and data area
			if ( memory_map_array[i].base_addr_low < BOOT_ADDRESS(&ebss) &&
				(memory_map_array[i].base_addr_low + memory_map_array[i].length_low) > BOOT_ADDRESS(&ebss) 
				)
			{
				memory_map_array[i].base_addr_low = BOOT_ADDRESS(&ebss);
			}
			
			//calculate virtual page array size
			total_virtual_pages =  memory_map_array[i].length_low / PAGE_SIZE;
			virtual_page_array_size = PAGE_ALIGN_UP ( total_virtual_pages * sizeof(VIRTUAL_PAGE) );
			total_size += virtual_page_array_size;
			//adjust total_virtual_pages
			total_virtual_pages = (memory_map_array[i].length_low - virtual_page_array_size) / PAGE_SIZE;

			//get physical address of the physical memory region
			pmr_pa = &ma_pa->physical_memory_regions[ma_pa->physical_memory_regions_count];
			
			pmr_pa->start_physical_address = memory_map_array[i].base_addr_low + virtual_page_array_size;
			pmr_pa->end_physical_address = memory_map_array[i].base_addr_low + memory_map_array[i].length_low;
			pmr_pa->virtual_page_array = (VIRTUAL_PAGE_PTR)memory_map_array[i].base_addr_low; 
			pmr_pa->virtual_page_count = total_virtual_pages;
						
			ma_pa->physical_memory_regions_count++;
		}
	}
	return total_size;
}

/*! Gets a free physical page for the kernel page table/directory use
	\return physical address
*/
static void * GetFreePhysicalPage()
{
	int i;
	MEMORY_AREA_PTR ma_pa;
	ma_pa = (MEMORY_AREA_PTR)BOOT_ADDRESS ( &memory_areas[0] );
	for(i=ma_pa->physical_memory_regions_count-1; i >= 0 ; i--)
	{
		PHYSICAL_MEMORY_REGION_PTR pmr_pa;
		void * pa;
		pmr_pa = &ma_pa->physical_memory_regions[i];
		
		//skip the region if there is not enough space 
		if ( (pmr_pa->end_physical_address - pmr_pa->start_physical_address) <= PAGE_SIZE )
			continue;
		//get the first page
		pa = (void *)pmr_pa->start_physical_address;
		
		//adjust the region
		pmr_pa->start_physical_address += PAGE_SIZE;
		pmr_pa->virtual_page_count--;
		
		memset(pa, 0, PAGE_SIZE);
		
		return pa;
	}
	return NULL;
}


/*! Create static page table entries for kernel
	Should not access any global variables directly; use BOOT_ADDRESS macro to get the correct address of the global varible to access.
	
	1) Initialize Kernel Page Table
		* Boot time kernel page directory contains only entry for the kernel code/data and virtual page array
		* It also creates pte for 0-4MB so that it is easy for the kernel to access 0-4MB during booting. 
		  This is also required because the stack is below 1MB
			Note this entry should removed after boot. To detect NULL pointer reference
		* It also creates entry for PT_SELF_MAP entry
		
	Each Page is 4MB size (no 2nd level page table needed)
	Page is global so it is not flushed on each task switch
		
	2) Set the control registers( CR3 and CR4) 
	3) Returns the correct value of CR0 in EAX register.
*/
static void InitKernelPageDirectory(UINT32 k_map_end)
{
	int i;
	UINT32 * k_page_dir = (UINT32 *)BOOT_ADDRESS( kernel_page_directory );
	UINT32 va, physical_address, end_physical_address;
	MEMORY_AREA_PTR ma_pa;
	
	/*initialize all kernel pages as invalid*/
	for(i=0; i<PAGE_DIRECTORY_ENTRIES; i++)
		k_page_dir[i]=0;

	/*	Enter mapping indentity mapping 0 - kernel end 
		and also enter mapping for VA 3GB to physical 0-kernel end
	*/
	va = KERNEL_VIRTUAL_ADDRESS_START;
	physical_address = 0;
	end_physical_address = BOOT_ADDRESS( k_map_end );
	do
	{
		//identity map
		EnterKernelPageTableEntry(physical_address, physical_address);
		//kernel code/data and also below 0 MB mapping
		EnterKernelPageTableEntry(va, physical_address);
		physical_address += PAGE_SIZE;
		va += PAGE_SIZE;
	}while( physical_address < end_physical_address );
	
	//map virtual page array
	ma_pa = (MEMORY_AREA_PTR)BOOT_ADDRESS ( &memory_areas[0] );
	for(i=0; i < ma_pa->physical_memory_regions_count; i++)
	{
		PHYSICAL_MEMORY_REGION_PTR pmr_pa;
		pmr_pa = &ma_pa->physical_memory_regions[i];
		
		physical_address = (UINT32)pmr_pa->virtual_page_array;
		do
		{
			EnterKernelPageTableEntry( va, physical_address);
			physical_address += PAGE_SIZE;
			va += PAGE_SIZE;		
		}while( physical_address <= ((UINT32)pmr_pa->virtual_page_array + (pmr_pa->virtual_page_count * sizeof(VIRTUAL_PAGE)) ) );
	}
	/*self mapping*/
	EnterKernelPageTableEntry( PT_SELF_MAP_ADDRESS, (UINT32)k_page_dir );
	
	/*activate paging*/	
	asm volatile(" movl %0, %%eax; movl %%eax, %%cr3; /*load cr3 with page directory address*/ \
                   movl %1, %%eax; movl %%eax, %%cr4; /*set cr4 for 4MB page size and global page*/"
                : 
                :"m"(k_page_dir), "c" (CR4_PAGE_SIZE_EXT | CR4_PAGE_GLOBAL_ENABLE) );
}

static void EnterKernelPageTableEntry(UINT32 va, UINT32 pa)
{
	int pd_index, pt_index;
	PAGE_TABLE_ENTRY_PTR page_table;
	PAGE_DIRECTORY_ENTRY_PTR k_page_dir;
	
	k_page_dir = (PAGE_DIRECTORY_ENTRY_PTR)BOOT_ADDRESS( kernel_page_directory );
	pd_index = PAGE_DIRECTORY_ENTRY_INDEX(va);
	pt_index = PAGE_TABLE_ENTRY_INDEX(va);
	
	//if we dont have a page table, create it
	if ( !k_page_dir[ pd_index ]._.present )
		k_page_dir[pd_index].all = ((UINT32)GetFreePhysicalPage()) | KERNEL_PTE_FLAG;
	
	//get the page table address
	page_table = (PAGE_TABLE_ENTRY_PTR) ( PFN_TO_PA(k_page_dir[ pd_index ]._.page_table_pfn) ) ;
	
	//enter pte in the page table.
	if ( !page_table[ pt_index ]._.present )
		page_table[pt_index].all = pa | KERNEL_PTE_FLAG;
}
/*! 	1) This phase will removes the unnessary page table entries that is created before enabling paging.
	2) Initializes the virtual page array.
*/
void InitPhysicalMemoryManagerPhaseII()
{
	int i;
	
	/*clear the PTE*/
	kernel_page_directory[0]=0;
	/*invalidate the TLB*/
	asm volatile("invlpg 0");
	for(i=0; i<memory_area_count; i++ )
	{
		int j;
		for(j=0; j<memory_areas[i].physical_memory_regions_count; j++ )
		{
			PHYSICAL_MEMORY_REGION_PTR pmr = &memory_areas[i].physical_memory_regions[j];
			pmr->virtual_page_array = (VIRTUAL_PAGE_PTR)BOOT_TO_KERNEL_ADDRESS(pmr->virtual_page_array);
			InitVirtualPageArray(pmr->virtual_page_array, pmr->virtual_page_count, pmr->start_physical_address);
		}
	}
}
