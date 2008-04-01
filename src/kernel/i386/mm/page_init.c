/*!
  \file	page_init.c
  \author	Samuel (samueldotj@gmail.com)
  \version 	3.0
  \date	
  			Created: 21-Mar-2008 5:13PM
  			Last modified: Tue Apr 01, 2008  11:10PM
  \brief	Ace Kernel memory management - i386 page directory/table management
	This file initializes the kernel page table it is in separate file because it executes in protected not in paged mode.
*/
#include <ace.h>
#include <kernel/i386/pagetab.h>
#include <kernel/vm/vm.h>
#include <kernel/debug.h>

extern UINT32 ebss;

#define BOOT_ADDRESS(addr)	(((UINT32)addr - KERNEL_VIRTUAL_ADDRESS_TEXT_START)+KERNEL_PHYSICAL_ADDRESS_LOAD )

#define KERNEL_PTE_FLAG		(PAGE_PRESENT | PAGE_READ_WRITE | PAGE_SUPERUSER | PAGE_4MB_SIZE | PAGE_GLOBAL)

/*! Create static page table entries for kernel
	Should not access any global variables directly; use BOOT_ADDRESS macro to get the correct address of the global varible to access.
	
	1) Initialize Kernel Page Table
		* Boot time kernel page directory contains only entry for the kernel code/data
		* It also creates pte for 0-4MB so that it is easy for the kernel to access 0-4MB during booting. 
		  This is also required because the stack is below 1MB
			Note this entry should removed after boot. To detect NULL pointer reference
		* It also creates entry for PT_SELF_MAP entry
		
	Each Page is 4MB size (no 2nd level page table needed)
	Page is global so it is not flushed on each task switch
		
	2) Set the control registers( CR3 and CR4) 
	3) Returns the correct value of CR0 in EAX register.
*/
void InitKernelPageDirectoryPhase1()
{
	int i;
	UINT32 * k_page_dir = (UINT32 *)BOOT_ADDRESS( kernel_page_directory );
	UINT32 k_map_end, physical_address, end_physical_address;

	/*calcaulation for initial mapping*/	
	k_map_end = (UINT32)&ebss; 		/*end of kernel text and data*/
	
	/*initialize all kernel pages as invalid*/
	for(i=0; i<PAGE_DIRECTORY_ENTRIES; i++)
		k_page_dir[i]=0;

	/*mapping for accessing below 0-4 MB*/
	k_page_dir[0] = KERNEL_PTE_FLAG;
	
	/*mapping 
		Logical address above 3gb (kernel code/data)
		Physical address start - 0
				        end - size of kernel/code/data.
	*/
	i = KERNEL_VIRTUAL_ADDRESS_START / (PAGE_TABLE_ENTRIES * PAGE_SIZE) ;
	physical_address = 0;
	end_physical_address = BOOT_ADDRESS( k_map_end );
	do
	{
		k_page_dir[i++] = physical_address | KERNEL_PTE_FLAG;
		physical_address += (PAGE_SIZE * PAGE_TABLE_ENTRIES);
	}while( physical_address < end_physical_address );

	/*self mapping*/
	k_page_dir[PT_SELF_MAP_INDEX] = ((UINT32)k_page_dir) | KERNEL_PTE_FLAG;
	
	/*activate paging*/	
	asm volatile(" movl %0, %%eax; movl %%eax, %%cr3; /*load cr3 with page directory address*/ \
                   movl %1, %%eax; movl %%eax, %%cr4; /*set cr4 for 4MB page size and global page*/"
                : 
                :"m"(k_page_dir), "c" (CR4_PAGE_SIZE_EXT | CR4_PAGE_GLOBAL_ENABLE) );
}

/*! This phase will removes the unnessary page table entries that is created before enabling paging.
*/
void InitKernelPageDirectoryPhase2()
{
	/*clear the PTE*/
	kernel_page_directory[0]=0;
	/*invalidate the TLB*/
	asm volatile("invlpg 0");
}
