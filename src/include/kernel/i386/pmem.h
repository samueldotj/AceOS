/*!
  \file		kernel/i386/pmem.h
  \brief	i386 page directory/table related macros
*/
#ifndef __PAGETAB__H
#define __PAGETAB__H

#include <ace.h>
#include <kernel/mm/vm.h>
#include <kernel/mm/pmem.h>

/*! Total page directory entries in i386*/
#define PAGE_DIRECTORY_ENTRIES		1024
/*! Total page table entries in i386*/
#define PAGE_TABLE_ENTRIES			1024

/*Page Table Entry flags*/
#define PAGE_PRESENT				1
#define PAGE_READ_WRITE				2
#define PAGE_SUPERUSER				4
#define PAGE_WT						8
#define PAGE_CACHE_DISABLE			16
#define PAGE_ACCESSED				32
#define PAGE_DIRTY					64
#define PAGE_4MB_SIZE				128
#define PAGE_GLOBAL					256

/*! kernel page table entry flag*/
#define KERNEL_PTE_FLAG				(PAGE_PRESENT | PAGE_READ_WRITE | PAGE_SUPERUSER | PAGE_GLOBAL)
/*! user page directory entry flag*/
#define USER_PDE_FLAG				(PAGE_PRESENT | PAGE_READ_WRITE | PAGE_SUPERUSER )
/*! user page table entry flag*/
#define USER_PTE_FLAG				(PAGE_PRESENT | PAGE_READ_WRITE )

#define CR3_PAGE_CACHE_DISABLE         
#define CR3_PAGE_WRITES_TRANSPARENT   

#define CR4_VM86_EXT                  1
#define CR4_VIRTUAL_INT               2
#define CR4_TIME_STAMP_DISABLE        4
#define CR4_DEBUG_EXT                 8
#define CR4_PAGE_SIZE_EXT             16
#define CR4_PHYSICAL_ADDRESS_EXT      32
#define CR4_MACHINE_CHECK_ENABLE      64
#define CR4_PAGE_GLOBAL_ENABLE        128
#define CR4_PERF_MONITOR_ENABLE       256

/*! kernel's physical address - kernel is loaded at 1MB - configured in kernel.ld*/
#define KERNEL_PHYSICAL_ADDRESS_LOAD		(0x100000)
/*since 4MB page size is used the page should be 4MB aligned -
so to map kernel and 0-1MB we are using kernel virtual address as the following*/
#define KERNEL_VIRTUAL_ADDRESS_START		(0xC0000000) 
#define KERNEL_VIRTUAL_ADDRESS_TEXT_START	(KERNEL_VIRTUAL_ADDRESS_START + KERNEL_PHYSICAL_ADDRESS_LOAD)

/*! Maxium kernel va size - 1gb*/
#define KERNEL_MAX_SIZE						(0x40000000)

/*! returns physical address for a given kernel virtual address*/
#define KERNEL_VTOP(k_addr)					( (k_addr) - KERNEL_VIRTUAL_ADDRESS_START + KERNEL_PHYSICAL_ADDRESS_LOAD )
/*! Converts the boot time physical address to kernel virtual address*/
#define BOOT_TO_KERNEL_ADDRESS(addr)		(((UINT32)addr + KERNEL_VIRTUAL_ADDRESS_TEXT_START)-KERNEL_PHYSICAL_ADDRESS_LOAD )

/*! page directory entry index for a given va*/
#define PAGE_DIRECTORY_ENTRY_INDEX(va)		( ((UINT32)va) >> 22 )
/*! page table entry index for a given va*/
#define PAGE_TABLE_ENTRY_INDEX(va)			( (((UINT32)va) >> PAGE_SHIFT ) & 0x03FF )

/*! Self map
one entry in the page table should point to itself so that it is easy to modify
the page tables = ((KERNEL_VIRTUAL_ADDRESS / (PAGE_TABLE_ENTRIES * PAGE_SIZE)) -1) */
#define PT_SELF_MAP_INDEX					(765)

/*! self map address*/
#define PT_SELF_MAP_ADDRESS					((UINT32)PT_SELF_MAP_INDEX * PAGE_TABLE_ENTRIES * PAGE_SIZE)

/*! Virtual address for page directory*/
#define PT_SELF_MAP_PAGE_DIRECTORY			( (PT_SELF_MAP_INDEX << 22) | (PT_SELF_MAP_INDEX << PAGE_SHIFT) )
/*! Virtual address for a page directory entry*/
#define PT_SELF_MAP_PAGE_DIRECTORY_PTR(va)	( &( (PAGE_DIRECTORY_ENTRY_PTR)PT_SELF_MAP_PAGE_DIRECTORY)[PAGE_DIRECTORY_ENTRY_INDEX(va)] )

/*Get the level 1 page table pointer va for a given va*/
#define PT_SELF_MAP_PAGE_TABLE1(va)			( (PT_SELF_MAP_INDEX << 22) | (PAGE_DIRECTORY_ENTRY_INDEX(va) << PAGE_SHIFT) )
/*Get the level 1 page table entry pointer va*/
#define PT_SELF_MAP_PAGE_TABLE1_PTE(va)		( &(((PAGE_TABLE_ENTRY_PTR)PT_SELF_MAP_PAGE_TABLE1(va))[PAGE_TABLE_ENTRY_INDEX(va)] ))

/*! Page frame number to physical address*/
#define PFN_TO_PA(pfn)						( ((UINT32)pfn) << PAGE_SHIFT )
/*! Physical address to Page frame number*/
#define PA_TO_PFN(pa)						( ((UINT32)pa) >> PAGE_SHIFT )

#ifdef __cplusplus
    extern "C" {
#endif

typedef struct virtual_address
{
	UINT32	offset:12,
			page_table_index:10,
			page_directory_index:10;
}VIRTUAL_ADDRESS, VIRTUAL_ADDRESS_PTR;

typedef union page_directory_entry
{
	UINT32	all;
	struct 
	{
		UINT32	
			present:1,
			write:1,
			supervisior:1,
			write_through:1,
			cache_disabled:1,
			accessed:1,
			reserved:1,
			page_size:1,
			global:1,
			software:3,
			page_table_pfn:20;
	};
}PAGE_DIRECTORY_ENTRY, * PAGE_DIRECTORY_ENTRY_PTR;

typedef union page_table_entry
{
	UINT32	all;
	struct 
	{
		UINT32	
			present:1,
			write:1,
			supervisior:1,
			write_through:1,
			cache_disabled:1,
			accessed:1,
			dirty:1,
			page_table_attribute_index:1,
			global:1,
			software:3,
			page_pfn:20;
	};
}PAGE_TABLE_ENTRY, * PAGE_TABLE_ENTRY_PTR;

struct physical_map
{	
	SPIN_LOCK					lock;
	PAGE_DIRECTORY_ENTRY_PTR 	page_directory;
};

extern PAGE_DIRECTORY_ENTRY kernel_page_directory[PAGE_DIRECTORY_ENTRIES] __attribute__ ((aligned (PAGE_SIZE)));
extern PHYSICAL_MAP kernel_physical_map;


#ifdef __cplusplus
	}
#endif

#endif
