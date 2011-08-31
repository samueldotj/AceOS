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
#include <kernel/pm/elf.h>

extern UINT32 ebss, kernel_code_start;

#define BOOT_ADDRESS(addr)	(((UINT32)addr - KERNEL_VIRTUAL_ADDRESS_TEXT_START)+KERNEL_PHYSICAL_ADDRESS_LOAD )

static UINT32 InitMemoryArea(MEMORY_AREA_PTR ma_pa, MULTIBOOT_MEMORY_MAP_PTR memory_map_array, int memory_map_count);
static void * GetFreePhysicalPage();
static void InitKernelPageDirectory();
static void EnterKernelPageTableEntry(UINT32 va, UINT32 pa, UINT32 prot);

/*the following contains where kernel code/data physical address start and end*/
UINT32 kernel_physical_address_start=KERNEL_PHYSICAL_ADDRESS_LOAD, kernel_physical_address_end=0;

/*! Initializes memory areas and kernel page directory.
 * \param magic - magic number passed by multiboot loader
 * \param mbi - multiboot information passed by multiboot loader
 */
void InitPhysicalMemoryManagerPhaseI(unsigned long magic, MULTIBOOT_INFO_PTR mbi)
{
	ELF32_SECTION_HEADER_PTR symbol_table_header, string_table_header;
	
	if ( magic != MULTIBOOT_BOOTLOADER_MAGIC )
		return;/*we will panic in main()*/
		
	/* get physical address of the memory area - currently no NUMA support for i386*/
	*((int *)BOOT_ADDRESS ( &memory_area_count ) ) = 1;
	
	*((UINT32 *)BOOT_ADDRESS( &kernel_reserve_range.code_pa_start ) ) = PAGE_ALIGN (  BOOT_ADDRESS(&kernel_code_start) );
	*((UINT32 *)BOOT_ADDRESS( &kernel_reserve_range.code_pa_end ) ) = PAGE_ALIGN_UP (  BOOT_ADDRESS(&ebss) );
	
	/* calculate the address range occupied by kernel modules */
	if ( mbi->flags & MB_FLAG_MODS )
	{
		MULTIBOOT_MODULE_PTR mod = (MULTIBOOT_MODULE_PTR)mbi->mods_addr;
		* ((UINT32 *)BOOT_ADDRESS( &kernel_reserve_range.module_pa_start ) ) = PAGE_ALIGN(mod->mod_start);
		* ((UINT32 *)BOOT_ADDRESS( &kernel_reserve_range.module_pa_end ) ) = PAGE_ALIGN_UP( mod->mod_end );
	}
	/* calculate the address range occupied by kernel symbol table*/
	if ( mbi->flags & MB_FLAG_ELF )
	{
		int i;
		ELF32_SECTION_HEADER_PTR sh = (ELF32_SECTION_HEADER_PTR)mbi->elf_sec.addr;
		/*loop to find symbol section header*/
		for( i=0; i<mbi->elf_sec.num; i++)
		{
			if ( sh[i].sh_type == SHT_SYMTAB )
			{
				/*store the symbol header address*/
				symbol_table_header = &sh[i];
				if ( sh[i].sh_link != SHN_UNDEF ) /*if the symbol table has string table store it*/
				{
					string_table_header = &sh[sh[i].sh_link];
					if ( string_table_header->sh_type != SHT_STRTAB )
						string_table_header = NULL;
				}
				/*if we couldnt find a string table, use the section header supplied string table*/
				if ( string_table_header == NULL )
					string_table_header = &sh[mbi->elf_sec.shndx];
				
				/*done so break the loop*/
				break;
			}
		}
		/*if symbol table found update it in the global data structure*/
		if( symbol_table_header )
		{
			* ((UINT32 *)BOOT_ADDRESS( &kernel_reserve_range.symbol_pa_start ) ) = symbol_table_header->sh_addr;
			* ((UINT32 *)BOOT_ADDRESS( &kernel_reserve_range.symbol_pa_end ) ) = ((UINT32)symbol_table_header->sh_addr)+symbol_table_header->sh_size;	
		}
		/*if string table found update it in the global data structure*/
		if( string_table_header )
		{
			* ((UINT32 *)BOOT_ADDRESS( &kernel_reserve_range.string_pa_start ) ) = string_table_header->sh_addr;
			* ((UINT32 *)BOOT_ADDRESS( &kernel_reserve_range.string_pa_end ) ) = ((UINT32)string_table_header->sh_addr)+string_table_header->sh_size;
		}
	}
	/*Initialize the memory area - calculate virtual page array*/
	InitMemoryArea((MEMORY_AREA_PTR) BOOT_ADDRESS(memory_areas), (MULTIBOOT_MEMORY_MAP_PTR)mbi->mmap_addr,  mbi->mmap_length / sizeof(MULTIBOOT_MEMORY_MAP) );
	
	/*Initialize kernel page directory and start paging*/
	InitKernelPageDirectory();
}

/*! Finds limit for virtual page array from both start and end
	VPA can be placed in either starting of a PMR or at end of a PMR
	
	VPA placement is based on the already used memory (used by bios/kernel symbols..) inside the pmr.
	
	Eg:
	In the following diagram 
		"#" denotes virtual pages
		"x" denotes physical pages managed by virtual pages
	
	1) VPA Placed at the beginning of a PMR:
	|PMR Start														PMR End |
	-------------------------------------------------------------------------
	|###############|xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx		
	VPA Start		VPA End
	
	2) VPA Placed at the end of a PMR:
	|PMR Start														PMR End |
	-------------------------------------------------------------------------
	xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx|###############|		
															VPA Start		VPA End

	\param pmr - physical memory region
	\param used_start - Starting address of a already used range
	\param used_end - Ending address of a already used range
	\param vpa_end_from_start - in/out param - Updated with the maximum possible unused address from start of pmr
	\param vpa_start_from_end - in/out param - Updated with the maximum possible unused address from end of pmr
*/
static void FindLimitForVpa(PHYSICAL_MEMORY_REGION_PTR pmr, UINT32 used_start, UINT32 used_end, UINT32 * vpa_end_from_start, UINT32 * vpa_start_from_end)
{
	/*case 1 - entire range used - we dont have any unused memory*/
	if ( used_start <= pmr->start_physical_address && used_end >= pmr->end_physical_address )
	{
		* vpa_end_from_start = pmr->start_physical_address;
		* vpa_start_from_end = pmr->end_physical_address;
	}
	/*case 2 - used range starts below the unused range and ends before pmr end - we have some unused memory in the end*/
	else if ( used_start <= pmr->start_physical_address && ( used_end > pmr->start_physical_address && used_end < pmr->end_physical_address ) )
	{
		* vpa_end_from_start = pmr->start_physical_address;
		if ( *vpa_start_from_end < used_end )
			*vpa_start_from_end = used_end;
	}
	/*case 3 - used range is somewhere in the pmr -	we have unused memory in either side
	*/
	else if ( used_start > pmr->start_physical_address && used_end < pmr->end_physical_address  )
	{
		/*find max space useable at the start*/
		if ( *vpa_end_from_start > used_start )
			*vpa_end_from_start = used_start;
		
		/*find max space useable at the end*/
		if ( *vpa_start_from_end < used_end )
			*vpa_start_from_end = used_end;
	}
	/*case 4 - used range starts some where inside the pmr range and spans beyond the pmr end - we have unused memory in the beginning*/
	else if ( (used_start > pmr->start_physical_address && used_start < pmr->end_physical_address)  && used_end > pmr->end_physical_address )
	{
		* vpa_start_from_end = pmr->end_physical_address;
		if ( *vpa_end_from_start > used_start )
			*vpa_end_from_start = used_start;
	}
}

/*! Intializes the memory area information - Helper function for InitPhysicalMemoryManagerPhaseI
 * \param ma_pa - pointer to memory area to initialize
 * \param memory_map_array - system memory map
 * \param memory_map_count - total memory maps in the system
 * \return size of all virtual page array in bytes
 */
static UINT32 InitMemoryArea(MEMORY_AREA_PTR ma_pa, MULTIBOOT_MEMORY_MAP_PTR memory_map_array, int memory_map_count)
{
	UINT32 total_size = 0;	/*total bytes occupied by virtual page array*/
	int i;
	
	ma_pa->physical_memory_regions_count = 0;
	for(i=0; i<memory_map_count && i<MAX_PHYSICAL_REGIONS; i++)
	{
		PHYSICAL_MEMORY_REGION_PTR pmr_pa;
		/*get physical address of the physical memory region*/
		pmr_pa = &ma_pa->physical_memory_regions[i];
		
		pmr_pa->start_physical_address = memory_map_array[i].base_addr_low;
		pmr_pa->end_physical_address = pmr_pa->start_physical_address + memory_map_array[i].length_low;
		pmr_pa->virtual_page_array = NULL; 
		pmr_pa->virtual_page_count = 0;
		pmr_pa->virtual_pages_used_for_booting = 0;
		pmr_pa->type = memory_map_array[i].type;
		
		ma_pa->physical_memory_regions_count++;
		/*create vm_page_array only for available RAM*/
		if( pmr_pa->type == PMEM_TYPE_AVAILABLE )
		{
			int total_virtual_pages, virtual_page_array_size;
			UINT32 region_size;
			UINT32 vpa_end_from_start, vpa_start_from_end;
			UINT32 usable_pa_start, usable_pa_end;
			
			/*we might need Real Mode IVT (Interrupt Vector Table)  and BDA (BIOS data area)  */
			if ( pmr_pa->start_physical_address < PAGE_SIZE )
				pmr_pa->start_physical_address = PAGE_SIZE;
			
			/*after the start physical address adjustment make sure the region is not out of memory*/
			if ( pmr_pa->start_physical_address < pmr_pa->end_physical_address )
			{
				region_size = PAGE_ALIGN(pmr_pa->end_physical_address - pmr_pa->start_physical_address);
				
				/*calculate virtual page array size*/
				total_virtual_pages =  region_size / PAGE_SIZE;
				virtual_page_array_size = PAGE_ALIGN_UP ( total_virtual_pages * sizeof(VIRTUAL_PAGE) );
				/*adjust total_virtual_pages*/
				total_virtual_pages = (region_size - virtual_page_array_size) / PAGE_SIZE;
				
				usable_pa_start = pmr_pa->end_physical_address;
				usable_pa_end = pmr_pa->start_physical_address;
				
				FindLimitForVpa( pmr_pa, *((VADDR *)BOOT_ADDRESS( &kernel_reserve_range.code_pa_start )), *((VADDR *)BOOT_ADDRESS( &kernel_reserve_range.code_pa_end )), &usable_pa_start, &usable_pa_end);
				FindLimitForVpa( pmr_pa, *((VADDR *)BOOT_ADDRESS( &kernel_reserve_range.module_pa_start )), *((VADDR *)BOOT_ADDRESS( &kernel_reserve_range.module_pa_end )), &usable_pa_start, &usable_pa_end);
				FindLimitForVpa( pmr_pa, *((VADDR *)BOOT_ADDRESS( &kernel_reserve_range.symbol_pa_start )), *((VADDR *)BOOT_ADDRESS( &kernel_reserve_range.symbol_pa_end )), &usable_pa_start, &usable_pa_end);
				FindLimitForVpa( pmr_pa, *((VADDR *)BOOT_ADDRESS( &kernel_reserve_range.string_pa_start )), *((VADDR *)BOOT_ADDRESS( &kernel_reserve_range.string_pa_end )), &usable_pa_start, &usable_pa_end);
				
				/*assume virutal page array will be placed at the beginning of the memory area and calculate its(vpa's) end address*/
				vpa_end_from_start = pmr_pa->start_physical_address + virtual_page_array_size;
				/*assume virutal page array will be placed at the end of the memory area and calculate its(vpa's) start address*/
				vpa_start_from_end = pmr_pa->end_physical_address - virtual_page_array_size;

				/*Assign starting physical address of virtual page array.
				  This physical address will be converted into virtual address by InitKernelPagediretory()
				*/
				if( vpa_end_from_start < usable_pa_start ) 
				{
					/*virtual page array is at the beginning of the region*/
					pmr_pa->virtual_page_array = (VIRTUAL_PAGE_PTR)pmr_pa->start_physical_address; 
					pmr_pa->start_physical_address = pmr_pa->start_physical_address + virtual_page_array_size;
				}
					
				else if ( vpa_start_from_end > usable_pa_end ) 
				{
					/*virtual page array is at the end of the region*/
					pmr_pa->virtual_page_array = (VIRTUAL_PAGE_PTR)vpa_start_from_end;
					pmr_pa->end_physical_address = pmr_pa->start_physical_address + (total_virtual_pages * PAGE_SIZE);
				}
				else 
				{
					/*not enough space for virtual page array*/
					pmr_pa->type = PMEM_TYPE_RESERVED;
				}
				pmr_pa->virtual_page_count = total_virtual_pages-1;
				
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
	ma_pa = (MEMORY_AREA_PTR)BOOT_ADDRESS( memory_areas );
	
	for(i=ma_pa->physical_memory_regions_count-1; i >= 0 ;i--)
	{
		PHYSICAL_MEMORY_REGION_PTR pmr_pa;
		void * pa;
		pmr_pa = &ma_pa->physical_memory_regions[i];
		
		/*if the region has enough free space allocate and return*/
		if ( pmr_pa->type == PMEM_TYPE_AVAILABLE && (pmr_pa->end_physical_address - pmr_pa->start_physical_address) > PAGE_SIZE )
		{
			/*get the last free page*/
			pa = (void *)(pmr_pa->end_physical_address - PAGE_SIZE - (pmr_pa->virtual_pages_used_for_booting * PAGE_SIZE));
		
			pmr_pa->virtual_pages_used_for_booting++;	

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
		* Boot time kernel page directory contains entry for the kernel code/data 
		* It also create entries for kernel moduels
		* It also create entries for kernel symbol table and string tables
		* It also create entries for virtual page array
		* It also creates pte for 0-4MB so that it is easy for the kernel to access 0-4MB during booting. 
		  This is also required because the stack is below 1MB
			Note this entry should removed after boot. To detect NULL pointer reference
		* It also creates entry for PT_SELF_MAP entry
		
	Each Page is 4MB size (no 2nd level page table needed)
	Page is global so it is not flushed on each task switch
		
	2) Set the control registers( CR3 and CR4) 
	3) Returns the correct value of CR0 in EAX register.
*/
static void InitKernelPageDirectory()
{
	int i;
	UINT32 * k_page_dir = (UINT32 *)BOOT_ADDRESS( kernel_page_directory );
	UINT32 va, physical_address, end_physical_address, module_start, kernel_symbol_table_start, kernel_symbol_table_size, kernel_string_table_start, kernel_string_table_size;
	MEMORY_AREA_PTR ma_pa;
	
	/*initialize all kernel pages as invalid*/
	for(i=0; i<PAGE_DIRECTORY_ENTRIES; i++)
		k_page_dir[i]=0;

	/*	Enter mapping indentity mapping 0 - kernel end (code/data)
		and also enter mapping for VA 3GB to physical 0-kernel end(code/data)
	*/
	va = KERNEL_VIRTUAL_ADDRESS_START;
	physical_address = 0;
	end_physical_address = *((VADDR *)BOOT_ADDRESS( &kernel_reserve_range.code_pa_end ));
	do
	{
		/*identity map*/
		EnterKernelPageTableEntry(physical_address, physical_address, PROT_READ | PROT_WRITE);
		/*kernel code/data and also below 0 MB mapping*/
		EnterKernelPageTableEntry(va, physical_address, PROT_READ | PROT_WRITE);
		physical_address += PAGE_SIZE;
		va += PAGE_SIZE;
	}while( physical_address < end_physical_address );
		
	/* Enter mapping for kernel modules*/
	module_start = PAGE_ALIGN( *((VADDR *)BOOT_ADDRESS( &kernel_reserve_range.module_pa_start)) );
	if ( module_start )
	{
		physical_address = module_start;
		end_physical_address = PAGE_ALIGN_UP( *((VADDR *)BOOT_ADDRESS( &kernel_reserve_range.module_pa_end)) );
		*((VADDR *)BOOT_ADDRESS( &kernel_reserve_range.module_va_start ) ) = va;
		do
		{
			EnterKernelPageTableEntry(va, physical_address, PROT_READ);
			physical_address += PAGE_SIZE;
			va += PAGE_SIZE;
		}while( physical_address < end_physical_address );
		*((UINT32 *)BOOT_ADDRESS( &kernel_reserve_range.module_va_end ) ) = va;
	}
	/* Enter mapping for kernel symbol table*/
	kernel_symbol_table_start = *((UINT32 *)BOOT_ADDRESS( &kernel_reserve_range.symbol_pa_start));
	kernel_symbol_table_size = *((UINT32 *)BOOT_ADDRESS( &kernel_reserve_range.symbol_pa_end)) - kernel_symbol_table_start;
	if ( kernel_symbol_table_start )
	{
		UINT32 corrected_va;
		/*symbol table may not page aligned address, so correct va*/
		corrected_va = va + (kernel_symbol_table_start-PAGE_ALIGN(kernel_symbol_table_start));
		* ((UINT32 *)BOOT_ADDRESS ( &kernel_reserve_range.symbol_va_start ) ) = corrected_va;
		* ((UINT32 *)BOOT_ADDRESS ( &kernel_reserve_range.symbol_va_end ) ) = corrected_va + kernel_symbol_table_size;
		physical_address = kernel_symbol_table_start;
		end_physical_address = PAGE_ALIGN_UP( * ((UINT32 *)BOOT_ADDRESS ( &kernel_reserve_range.symbol_pa_end ) ) );
		do
		{
			EnterKernelPageTableEntry(va, physical_address, PROT_READ);
			physical_address += PAGE_SIZE;
			va += PAGE_SIZE;
		}while( physical_address < end_physical_address );
	}
	/* Enter mapping for kernel string table*/
	kernel_string_table_start = *((UINT32 *)BOOT_ADDRESS( &kernel_reserve_range.string_pa_start));
	kernel_string_table_size = *((UINT32 *)BOOT_ADDRESS( &kernel_reserve_range.string_pa_end)) - kernel_string_table_start;
	if ( kernel_string_table_start )
	{
		UINT32 corrected_va;
		/*string table may not page aligned address, so correct va*/
		corrected_va = va + (kernel_string_table_start-PAGE_ALIGN(kernel_string_table_start));
		*((UINT32 *)BOOT_ADDRESS( &kernel_reserve_range.string_va_start)) = corrected_va;
		*((UINT32 *)BOOT_ADDRESS( &kernel_reserve_range.string_va_end)) = corrected_va + kernel_string_table_size;
		physical_address = kernel_string_table_start;
		end_physical_address = PAGE_ALIGN_UP( *((UINT32 *)BOOT_ADDRESS( &kernel_reserve_range.string_pa_end)) );
		do
		{
			EnterKernelPageTableEntry(va, physical_address, PROT_READ);
			physical_address += PAGE_SIZE;
			va += PAGE_SIZE;
		}while( physical_address < end_physical_address );
	}

	/*map virtual page array*/
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
				EnterKernelPageTableEntry( va, physical_address, PROT_READ | PROT_WRITE);
				physical_address += PAGE_SIZE;
				va += PAGE_SIZE;
			}while( physical_address < ( end_address ) );
		}
	}
	
	/*Update the kernel free VA start address*/
	*((UINT32 *)BOOT_ADDRESS( &kernel_reserve_range.kmem_va_start ) ) = va;
	
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
static void EnterKernelPageTableEntry(UINT32 va, UINT32 pa, UINT32 prot)
{
	int pd_index, pt_index;
	PAGE_TABLE_ENTRY_PTR page_table;
	PAGE_DIRECTORY_ENTRY_PTR k_page_dir;
	
	k_page_dir = (PAGE_DIRECTORY_ENTRY_PTR)BOOT_ADDRESS( kernel_page_directory );
	pd_index = PAGE_DIRECTORY_ENTRY_INDEX(va);
	pt_index = PAGE_TABLE_ENTRY_INDEX(va);
	
	/*if we dont have a page table, create it*/
	if ( !k_page_dir[ pd_index ].present )
	{
		UINT32 pa = (UINT32)GetFreePhysicalPage();
		page_table = (PAGE_TABLE_ENTRY_PTR) pa;
		
		/*enter pde*/
		k_page_dir[pd_index].present = 1;
		k_page_dir[pd_index].global = 1;
		if (prot & PROT_WRITE)
			k_page_dir[pd_index].write = 1;
		k_page_dir[pd_index].page_table_pfn = PA_TO_PFN(pa);
	}
	else
	{
		/*get the page table address*/
		page_table = (PAGE_TABLE_ENTRY_PTR) ( PFN_TO_PA(k_page_dir[ pd_index ].page_table_pfn) ) ;
	}
	
	/*enter pte in the page table.*/
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
	kernel_physical_map.virtual_map = &kernel_map;
	
	/*initialize the virtual page array*/
	for(i=0; i<memory_area_count; i++ )
	{
		int j;
		ktrace("System map:    START     END       PAGES      TYPE\n");
		for(j=0; j<memory_areas[i].physical_memory_regions_count; j++ )
		{
			PHYSICAL_MEMORY_REGION_PTR pmr = &memory_areas[i].physical_memory_regions[j];
			ktrace("           %9p %9p %9d %10s\n", pmr->start_physical_address, pmr->end_physical_address, pmr->virtual_page_count, 
				pmr->type == PMEM_TYPE_AVAILABLE ? "Available" : pmr->type == PMEM_TYPE_ACPI_RECLAIM ? "ACPI Reclaim" : pmr->type == PMEM_TYPE_ACPI_NVS ? "ACPI NVS" : "Reserved" );
			
			if ( pmr->type == PMEM_TYPE_AVAILABLE )
			{
				vm_data.total_memory_pages += InitVirtualPageArray(pmr->virtual_page_array, pmr->virtual_page_count, pmr->virtual_page_count - pmr->virtual_pages_used_for_booting, pmr->start_physical_address);
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
