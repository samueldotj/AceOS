/*!
	\file	src/kernel/i386/mm/pmem_init.c	
	\brief	Physical memory manager initialization routines
	\note	all these routines uses physical memory and dont know about va.
*/
#include <string.h>
#include <kernel/multiboot.h>
#include <kernel/debug.h>
#include <kernel/arch.h>
#include <kernel/mm/vm.h>
#include <kernel/mm/virtual_page.h>
#include <kernel/mm/pmem.h>
#include <kernel/i386/pmem.h>

extern UINT32 ebss, kernel_code_start;

#define BOOT_ADDRESS(addr)	(((UINT32)addr - KERNEL_VIRTUAL_ADDRESS_TEXT_START)+KERNEL_PHYSICAL_ADDRESS_LOAD )

static UINT32 InitMemoryArea(UINT32 kernel_start, UINT32 kernel_end, UINT32 mb_module_start_address, UINT32 mb_module_end_address, 
						MEMORY_AREA_PTR ma_pa, MULTIBOOT_MEMORY_MAP_PTR memory_map_array, int memory_map_count);
static void * GetFreePhysicalPage();
static void InitKernelPageDirectory(UINT32 k_map_end, UINT32 mb_module_start_address, UINT32 mb_module_end_address);
static void EnterKernelPageTableEntry(UINT32 va, UINT32 pa);

/*! Initializes memory areas and kernel page directory.
 * \param magic - magic number passed by multiboot loader
 * \param mbi - multiboot information passed by multiboot loader
 */
void InitPhysicalMemoryManagerPhaseI(unsigned long magic, MULTIBOOT_INFO_PTR mbi)
{
	UINT32 vpa_size;
	UINT32 mb_module_start_address = 0;
	UINT32 mb_module_end_address = 0;
	UINT32 kernel_start = PAGE_ALIGN_UP (  BOOT_ADDRESS(&kernel_code_start) );
	UINT32 kernel_end = PAGE_ALIGN_UP (  BOOT_ADDRESS(&ebss) );
	
	if ( magic != MULTIBOOT_BOOTLOADER_MAGIC )
		return;/*we will panic in main()*/
		
	/* get physical address of the memory area - currently no NUMA support for i386*/
	* ((int *)BOOT_ADDRESS ( &memory_area_count ) )  = 1;
	/* calculate the address range occupied by kernel modules */
	if ( mbi->flags & MB_FLAG_MODS )
	{
		MULTIBOOT_MODULE_PTR mod = (MULTIBOOT_MODULE_PTR)mbi->mods_addr;
		mb_module_start_address = mod->mod_start;
		mb_module_end_address = PAGE_ALIGN_UP( mod->mod_end);
	}
	vpa_size = InitMemoryArea(kernel_start, kernel_end, mb_module_start_address, mb_module_end_address, (MEMORY_AREA_PTR) BOOT_ADDRESS(memory_areas), (MULTIBOOT_MEMORY_MAP_PTR)mbi->mmap_addr,  mbi->mmap_length / sizeof(MULTIBOOT_MEMORY_MAP) );
	
	InitKernelPageDirectory( PAGE_ALIGN_UP(&ebss), mb_module_start_address, mb_module_end_address );
}

/*! Intializes the memory area information - Helper function for InitPhysicalMemoryManagerPhaseI
 * \param ma_pa - pointer to memory area to initialize
 * \param memory_map_array - system memory map
 * \param memory_map_count - total memory maps in the system
 * \return size of all virtual page array in bytes
 */
static UINT32 InitMemoryArea(UINT32 kernel_start, UINT32 kernel_end, UINT32 mb_module_start_address, UINT32 mb_module_end_address, 
						MEMORY_AREA_PTR ma_pa, MULTIBOOT_MEMORY_MAP_PTR memory_map_array, int memory_map_count)
{
	UINT32 total_size = 0;	//total bytes occupied by virtual page array
	UINT32 virtual_page_array_start, virtual_page_array_end;
	int i;
	
	ma_pa->physical_memory_regions_count = 0;
	for(i=0; i<memory_map_count && i<MAX_PHYSICAL_REGIONS; i++)
	{
		PHYSICAL_MEMORY_REGION_PTR pmr_pa;
		//get physical address of the physical memory region
		pmr_pa = &ma_pa->physical_memory_regions[i];
		
		pmr_pa->start_physical_address = memory_map_array[i].base_addr_low;
		pmr_pa->end_physical_address = pmr_pa->start_physical_address + memory_map_array[i].length_low;
		pmr_pa->virtual_page_array = NULL; 
		pmr_pa->virtual_page_count = 0;
		pmr_pa->type = memory_map_array[i].type;
		
		ma_pa->physical_memory_regions_count++;
		//create vm_page_array only for available RAM
		if( pmr_pa->type == PMEM_TYPE_AVAILABLE )
		{
			int total_virtual_pages, virtual_page_array_size;
			UINT32 region_size;
			
			//we might need Real Mode IVT (Interrupt Vector Table)  and BDA (BIOS data area)  
			if ( pmr_pa->start_physical_address < PAGE_SIZE )
				pmr_pa->start_physical_address = PAGE_SIZE;
			
recalculate:
			/*after the start physical address adjustment make sure the region is not out of memory*/
			if ( pmr_pa->start_physical_address < pmr_pa->end_physical_address )
			{
			
				region_size = pmr_pa->end_physical_address - pmr_pa->start_physical_address;
				
				//calculate virtual page array size
				total_virtual_pages =  region_size / PAGE_SIZE;
				virtual_page_array_size = PAGE_ALIGN_UP ( total_virtual_pages * sizeof(VIRTUAL_PAGE) );
				//adjust total_virtual_pages
				total_virtual_pages = (region_size - virtual_page_array_size) / PAGE_SIZE;
				
				/*this physical address will be converted into virtual address by InitKernelPagediretory()*/
				pmr_pa->virtual_page_array = (VIRTUAL_PAGE_PTR)pmr_pa->start_physical_address; 
				virtual_page_array_start = pmr_pa->start_physical_address;
				virtual_page_array_end = pmr_pa->start_physical_address + virtual_page_array_size;

				/*adjust the region start if kernel code and data is loaded in the calcualted virtual page array meta area*/
				if ( ( virtual_page_array_start <=  kernel_start && virtual_page_array_end > kernel_start ) ||
					( virtual_page_array_start >=  kernel_start && virtual_page_array_start < kernel_end ) )
				{
					pmr_pa->start_physical_address = kernel_end;
					goto recalculate;
				}

				/*adjust the region start if kernel modules are loaded in the calcualted virtual page array meta area*/
				if ( ( virtual_page_array_start <=  mb_module_start_address && virtual_page_array_end > mb_module_start_address ) ||
					( virtual_page_array_start >=  mb_module_start_address && virtual_page_array_start < mb_module_end_address) )
				{
					pmr_pa->start_physical_address = mb_module_end_address;
					goto recalculate;
				}
				
				/*managed physical address right after the virtual page array*/
				pmr_pa->start_physical_address = ((UINT32)pmr_pa->virtual_page_array) + virtual_page_array_size;
				pmr_pa->virtual_page_count = total_virtual_pages;
				
				total_size += virtual_page_array_size;
			}
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
		
		//if the region has enough free space allocate and return
		if ( pmr_pa->type == PMEM_TYPE_AVAILABLE && (pmr_pa->end_physical_address - pmr_pa->start_physical_address) > PAGE_SIZE )
		{
			//get the last page
			pa = (void *)(pmr_pa->end_physical_address-PAGE_SIZE);
			
			//adjust the region
			pmr_pa->end_physical_address = (UINT32)pa;
			pmr_pa->virtual_page_count--;
			
			memset(pa, 0, PAGE_SIZE);
			
			return pa;
		}
	}
	/*panic if we dont find a free physical page- however we cant call panic() because we havent boot yet so just halt*/
	asm("cli;hlt");
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
static void InitKernelPageDirectory(UINT32 k_map_end, UINT32 mb_module_start_address, UINT32 mb_module_end_address)
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
	
	/* Enter mapping for kernel modules*/
	if ( mb_module_start_address )
	{
		*((VADDR *)BOOT_ADDRESS( &multiboot_module_va_start ) ) = va;
		physical_address = mb_module_start_address;
		end_physical_address = mb_module_end_address;
		do
		{
			EnterKernelPageTableEntry(va, physical_address);
			physical_address += PAGE_SIZE;
			va += PAGE_SIZE;
		}while( physical_address < end_physical_address );
		*((VADDR *)BOOT_ADDRESS( &multiboot_module_va_end ) ) = va;
	}
	
	//map virtual page array
	ma_pa = (MEMORY_AREA_PTR) BOOT_ADDRESS( &memory_areas[0] );
	for(i=0; i < ma_pa->physical_memory_regions_count; i++)
	{
		UINT32 end_address;
		PHYSICAL_MEMORY_REGION_PTR pmr_pa;
		pmr_pa = &ma_pa->physical_memory_regions[i];
		
		if ( pmr_pa->type == PMEM_TYPE_AVAILABLE )
		{
			physical_address = (UINT32)pmr_pa->virtual_page_array;
			end_address = physical_address + (pmr_pa->virtual_page_count * sizeof(VIRTUAL_PAGE));
			pmr_pa->virtual_page_array =  (VIRTUAL_PAGE_PTR)va;
			do
			{
				EnterKernelPageTableEntry( va, physical_address);
				physical_address += PAGE_SIZE;
				va += PAGE_SIZE;
			}while( physical_address <= ( end_address ) );
		}
	}
	
	*((UINT32 *)BOOT_ADDRESS( &kernel_free_virtual_address ) ) = va;
	
	/*self mapping*/
	k_page_dir[PT_SELF_MAP_INDEX] = ((UINT32)k_page_dir) | KERNEL_PTE_FLAG;
	
	/*activate paging*/	
	asm volatile(" movl %0, %%eax; movl %%eax, %%cr3; /*load cr3 with page directory address*/ \
                   movl %1, %%eax; movl %%eax, %%cr4; /*set cr4 for 4MB page size and global page*/"
                : 
                :"m"(k_page_dir), "c" (CR4_PAGE_SIZE_EXT | CR4_PAGE_GLOBAL_ENABLE) );
}

/*! Helper function to enter kernel page table entry during boot
	\param va - virtual address for which translation needs to be added
	\param pa - physical address needs to be filled
*/
static void EnterKernelPageTableEntry(UINT32 va, UINT32 pa)
{
	int pd_index, pt_index;
	PAGE_TABLE_ENTRY_PTR page_table;
	PAGE_DIRECTORY_ENTRY_PTR k_page_dir;
	
	k_page_dir = (PAGE_DIRECTORY_ENTRY_PTR)BOOT_ADDRESS( kernel_page_directory );
	pd_index = PAGE_DIRECTORY_ENTRY_INDEX(va);
	pt_index = PAGE_TABLE_ENTRY_INDEX(va);
	
	//if we dont have a page table, create it
	if ( !k_page_dir[ pd_index ].present )
	{
		UINT32 pa = (UINT32)GetFreePhysicalPage();
		page_table = (PAGE_TABLE_ENTRY_PTR) pa;
		
		/*enter pde*/
		k_page_dir[pd_index].all = KERNEL_PTE_FLAG;
		k_page_dir[pd_index].page_table_pfn = PA_TO_PFN(pa);
	}
	else
	{
		//get the page table address
		page_table = (PAGE_TABLE_ENTRY_PTR) ( PFN_TO_PA(k_page_dir[ pd_index ].page_table_pfn) ) ;
	}
	
	//enter pte in the page table.
	if ( !page_table[ pt_index ].present )
	{
		page_table[pt_index].all = KERNEL_PTE_FLAG;
		page_table[pt_index].page_pfn =  PA_TO_PFN(pa);
	}
}

/*! Initializes the Physical Memory Manager in Virtual Address mode
	1) Initializes the virtual page array.
*/
void InitPhysicalMemoryManagerPhaseII()
{
	int i;
	
	/*initialize the kernel physical map*/
	InitSpinLock( &kernel_physical_map.lock );
	kernel_physical_map.page_directory = kernel_page_directory;
	
	/*initialize the virtual page array*/
	for(i=0; i<memory_area_count; i++ )
	{
		int j;
		kprintf("System map:    START     END       PAGES      TYPE\n");
		for(j=0; j<memory_areas[i].physical_memory_regions_count; j++ )
		{
			PHYSICAL_MEMORY_REGION_PTR pmr = &memory_areas[i].physical_memory_regions[j];
			kprintf("           %9p %9p %9d %10s\n", pmr->start_physical_address, pmr->end_physical_address, pmr->virtual_page_count, 
				pmr->type == PMEM_TYPE_AVAILABLE ? "Available" : pmr->type == PMEM_TYPE_ACPI_RECLAIM ? "ACPI Reclaim" : pmr->type == PMEM_TYPE_ACPI_NVS ? "ACPI NVS" : "Reserved" );
			
			if ( pmr->type == PMEM_TYPE_AVAILABLE )
			{
				vm_data.total_memory_pages += InitVirtualPageArray(pmr->virtual_page_array, pmr->virtual_page_count, pmr->start_physical_address);
			}
		}
	}
	
	vm_data.total_free_pages = vm_data.total_memory_pages;
}
/*! Completes the initialization of Physical memory manager by removing unncessary pte entries
 **/
void CompletePhysicalMemoryManagerInit()
{
	/*clear the boot PTE*/
	kernel_page_directory[0].all = 0;
	/*invalidate the TLB*/
	asm volatile("invlpg 0");
}
