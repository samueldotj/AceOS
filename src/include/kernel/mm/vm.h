/*!
  \file	vm.h
  \author	Samuel (samueldotj@gmail.com)
  \version 	3.0
  \date	
  			Created: 21-Mar-2008 5:13PM
  			Last modified: 
  \brief	virtual memory macros and functions
*/
#ifndef __VM__H
#define __VM__H

#include <ace.h>
#include <ds/list.h>
#include <ds/avl_tree.h>
#include <sync/spinlock.h>
#include <kernel/error.h>
#include <kernel/mm/vm_types.h>

#if	ARCH == i386
	#define PAGE_SIZE					4096
	#define PAGE_SHIFT					12

	#define KERNEL_MAP_START_VA			(0xC0000000)
	#define KERNEL_MAP_END_VA			((UINT32)-1)
	
	#define USER_MAP_START_VA			(1024*1024)
	#define USER_MAP_END_VA				(KERNEL_MAP_START_VA-1)
		
#endif

#define PAGE_ALIGN(addr)			((UINT32)(addr) & -PAGE_SIZE)
#define PAGE_ALIGN_UP(addr)			((UINT32)((addr) + PAGE_SIZE - 1) & -PAGE_SIZE)

#define PAGE_ALIGN_4MB(addr)		((UINT32)(addr) & -(4096*1024))
#define PAGE_ALIGN_UP_4MB(addr)		PAGE_ALIGN_4MB( (addr) + (1024*1024) - 1 )

#define NUMBER_OF_PAGES(size)		(PAGE_ALIGN_UP(size) >> PAGE_SHIFT)

#define VA_RANGE_SEARCH_FROM_TOP	1

#define PROT_READ					1
#define PROT_WRITE					2

struct vm_data
{
	SPIN_LOCK			lock;					//lock for entire structure
	
	UINT32				total_memory_pages;		//total system memory in PAGE_SIZE unit
	UINT32				total_free_pages;		//total free memory in PAGE_SIZE unit

	AVL_TREE_PTR		free_tree;				//free virtual page ranges
	
	VIRTUAL_PAGE_PTR	active_list;			//points to the first page in the active list
	VIRTUAL_PAGE_PTR	inactive_list;			//points to the first page in the active list
};

struct vm_protection 
{
	int user_write:1,
		user_read:1,
		kernel_write:1,
		kernel_read:1;
};

struct virtual_map
{
	SPIN_LOCK			lock;				//lock for the entire structure
	int					reference_count;	//total number of references
	
	UINT32				start;				//start virtual address
	UINT32				end;				//end virtual address
	
	AVL_TREE_PTR		descriptors;		//root of the descriptor tree
	UINT32 				descriptor_count;	//total number of descriptors
	
	PHYSICAL_MAP_PTR	physical_map;		//pointer to the physical map
};

struct vm_descriptor
{
	SPIN_LOCK			lock;				//lock for the entire structure
	int					reference_count;	//total number of references
	
	VIRTUAL_MAP_PTR		virtual_map;		//pointer to the virtual map
	AVL_TREE			tree_node;			//avl tree for all descriptor in the map
	
	UINT32				start;				//start virtual address
	UINT32				end;				//end virtual address
	
	VM_PROTECTION		protection;			//protection for this range
	
	VM_UNIT_PTR			unit;				//pointer to the vm_unit
};

struct vm_unit
{
	SPIN_LOCK			lock;				//lock for the entire structure
	int					reference_count;	//total number of references
	
	UINT32				type;				//type - text, code, heap...
	UINT32				flag;				//flag - shared, private....
		
	UINT32				size;				//total size of this unit
	
	SPIN_LOCK			vtop_lock;			//lock to protect array and page_count
	VM_VTOP_PTR			vtop_array;			//pointer to the virtual page array
	int					page_count;			//to pages in memory
};

struct vm_vtop 
{
	union
	{
		int 				in_memory;		//if 1 means it is in memory
		VIRTUAL_PAGE_PTR	vpage;			//pointer to the virtual page
	}p;
};

extern VM_DATA vm_data;
extern VIRTUAL_MAP kernel_map;

extern VADDR kernel_free_virtual_address;

extern VM_PROTECTION protection_kernel_write;
extern VM_PROTECTION protection_kernel_read;
extern VM_PROTECTION protection_user_write;
extern VM_PROTECTION protection_user_read;

#ifdef __cplusplus
    extern "C" {
#endif

void InitVm();

void InitVmDescriptor(VM_DESCRIPTOR_PTR descriptor, VIRTUAL_MAP_PTR vmap, VADDR start, VADDR end, VM_UNIT_PTR vm_unit, VM_PROTECTION_PTR protection);
VM_DESCRIPTOR_PTR CreateVmDescriptor(VIRTUAL_MAP_PTR vmap, VADDR start, VADDR end, VM_UNIT_PTR vm_unit, VM_PROTECTION_PTR protection);
void * FindFreeVmRange(VIRTUAL_MAP_PTR vmap, VADDR start, UINT32 size, UINT32 option);

VM_UNIT_PTR CreateVmUnit(UINT32 type, UINT32 size);

ERROR_CODE AllocateVirtualMemory(VIRTUAL_MAP_PTR vmap, VADDR * va_ptr, VADDR preferred_start, UINT32 size, UINT32 protection, UINT32 flags);
ERROR_CODE FreeVirtualMemory(VIRTUAL_MAP_PTR vmap, VADDR va, UINT32 size, UINT32 flags);

VADDR MapPhysicalMemory(VIRTUAL_MAP_PTR vmap, UINT32 pa, UINT32 size);

#ifdef __cplusplus
	}
#endif

#endif

