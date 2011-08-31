/*!
  \file		kernel/mm/vm.h
  \brief	virtual memory macros and functions
*/
#ifndef __VM__H
#define __VM__H

#include <ace.h>
#include <ds/list.h>
#include <ds/avl_tree.h>
#include <sync/spinlock.h>
#include <kernel/error.h>
#include <kernel/mm/kmem.h>
#include <kernel/mm/vm_types.h>
#include <kernel/vfs/vfs_types.h>

#if	ARCH == i386
	#define PAGE_SIZE					4096
	#define PAGE_SHIFT					12

	#define KERNEL_MAP_START_VA			(0xC0000000)
	#define KERNEL_MAP_END_VA			((UINT32)-1)
	
	#define USER_MAP_START_VA			(PAGE_SIZE)
	#define USER_MAP_END_VA				(KERNEL_MAP_START_VA-1)
		
#endif

/*! aligns a address to page lower boundary*/
#define PAGE_ALIGN(addr)			((UINT32)(addr) & -PAGE_SIZE)
/*! aligns a address to page upper boundary*/
#define PAGE_ALIGN_UP(addr)			((UINT32)((addr) + PAGE_SIZE - 1) & -PAGE_SIZE)

#define PAGE_ALIGN_4MB(addr)		((UINT32)(addr) & -(4096*1024))
#define PAGE_ALIGN_UP_4MB(addr)		PAGE_ALIGN_4MB( (addr) + (1024*1024) - 1 )

#define PAGE_MASK					( ~(PAGE_SIZE-1) )

#define IS_PAGE_ALIGNED(addr)		( !( ((unsigned long)addr) & (PAGE_SIZE-1) ) )

/*! Total number pages required by given size in bytes*/
#define NUMBER_OF_PAGES(size)		(PAGE_ALIGN_UP(size) >> PAGE_SHIFT)

/*! When searching for free virtual range start from 0*/
#define VA_RANGE_SEARCH_FROM_TOP	1

#define PROT_READ					1
#define PROT_WRITE					2
#define PROT_EXECUTE				4

typedef enum 
{
	VM_UNIT_TYPE_KERNEL=1,				/*kernel mapping*/
	VM_UNIT_TYPE_ANONYMOUS,				
	VM_UNIT_TYPE_FILE_MAPPED,
	VM_UNIT_TYPE_STACK,
	VM_UNIT_TYPE_PTE
}VM_UNIT_TYPE;

typedef enum
{
	VM_UNIT_FLAG_SHARED=1,
	VM_UNIT_FLAG_PRIVATE
}VM_UNIT_FLAG;

/*! structure to contain VM data for a NUMA node*/
struct vm_data
{
	SPIN_LOCK			lock;					/*! lock for entire structure*/
	
	UINT32				total_memory_pages;		/*! total system memory in PAGE_SIZE unit*/
	UINT32				total_free_pages;		/*! total free memory in PAGE_SIZE unit*/

	AVL_TREE_PTR		free_tree;				/*! free virtual page ranges*/
	AVL_TREE_PTR		free_tree_1M;			/*! free virtual page ranges under 1 M*/
	AVL_TREE_PTR		free_tree_16M;			/*! free virtual page ranges under 16 M*/
	
	VIRTUAL_PAGE_PTR	active_list;			/*! points to the first page in the active list*/
	VIRTUAL_PAGE_PTR	inactive_list;			/*! points to the first page in the active list*/
};

struct vm_protection 
{
	int user_write:1,
		user_read:1,
		kernel_write:1,
		kernel_read:1;
};

/*! structure to contain virtual mapping details for a task*/
struct virtual_map
{
	SPIN_LOCK			lock;				/*! lock for the entire structure*/
	int					reference_count;	/*! total number of references*/
	
	VADDR				start;				/*! start virtual address of this map*/
	VADDR				end;				/*! end virtual address of this map*/
	
	AVL_TREE_PTR		descriptors;		/*! root of the descriptor tree*/
	UINT32 				descriptor_count;	/*! total number of descriptors*/
	
	PHYSICAL_MAP_PTR	physical_map;		/*! pointer to the physical map*/
};

/*! contains valid virtual address ranges of a task*/
struct vm_descriptor
{
	SPIN_LOCK			lock;				/*! lock for the entire structure*/
	int					reference_count;	/*! total number of references*/
	
	VIRTUAL_MAP_PTR		virtual_map;		/*! pointer to the virtual map*/
	AVL_TREE			tree_node;			/*! avl tree for all descriptor in the map*/
	
	VADDR				start;				/*! start virtual address*/
	VADDR				end;				/*! end virtual address*/
	
	VADDR				offset_in_unit;		/*! starting offset in the vm unit*/
	
	UINT32				protection;			/*! protection for this range*/
	
	VM_UNIT_PTR			unit;				/*! pointer to the vm_unit*/
};

/*! contains details about a valid virtual address range*/
struct vm_unit
{
	SPIN_LOCK			lock;				/*! lock for the entire structure*/
	int					reference_count;	/*! total number of references*/
	
	VM_UNIT_TYPE		type;				/*! type - text, code, heap...*/
	VM_UNIT_FLAG		flag;				/*! flag - shared, private....*/
		
	UINT32				size;				/*! total size of this unit*/
	
	SPIN_LOCK			vtop_lock;			/*! lock to protect array and page_count*/
	VM_VTOP_PTR			vtop_array;			/*! pointer to the virtual page array*/
	int					page_count;			/*! total pages in memory*/
	
	VNODE_PTR			vnode;				/*! pointer to vnode, if this unit backed by a file/swap*/
	UINT32				offset;				/*! offset in the file*/
	LIST				units_in_vnode_list;/*! all the vmunits corresponds to a vm unit are linked through this list*/
};

struct vm_vtop 
{
	union
	{
		int 				in_memory;		/*! if 1 means it is in memory else in filesystem/swap*/
		VIRTUAL_PAGE_PTR	vpage;			/*! pointer to the virtual page if the page is in memory*/
		VNODE_PTR			vnode;			/*! pointer to the vnode if the page is in filesystem/swap*/
	};
};

/*! kernel_reserve_range - Virtual and physical address ranges reserved by kernel
		Arch depended pmem_init routines updates the pa
		Vm init routines uses the pa and updates the va and creates va to pa mapping
*/
struct kernel_reserve_range
{
	VADDR code_pa_start,	code_pa_end;		/* kernel text*/
	VADDR data_pa_start,	data_pa_end;		/* kernel data*/
	VADDR module_pa_start,	module_pa_end;		/* kernel module*/
	VADDR symbol_pa_start,	symbol_pa_end;		/* kernel symbol table*/
	VADDR string_pa_start,	string_pa_end;		/* kernel string table*/
	
	VADDR code_va_start,	code_va_end;		/* kernel text*/
	VADDR data_va_start,	data_va_end;		/* kernel data*/
	VADDR module_va_start, 	module_va_end;		/* kernel module*/
	VADDR symbol_va_start, 	symbol_va_end;		/* kernel symbol table*/
	VADDR string_va_start, 	string_va_end;		/* kernel string table*/
	
	VADDR kmem_va_start, 	kmem_va_end;		/* kernel malloc space*/
};

typedef enum mm_fault_type
{
	MM_FAULT_TYPE_NONE=0,
	MM_FAULT_TYPE_READ=1,
	MM_FAULT_TYPE_WRITE=2,
	MM_FAULT_TYPE_EXECUTE=4
}MM_FAULT_TYPE, * MM_FAULT_TYPE_PTR;

extern VM_DATA vm_data;
extern VIRTUAL_MAP kernel_map;

extern KERNEL_RESERVE_RANGE kernel_reserve_range;

extern VM_PROTECTION protection_kernel_write;
extern VM_PROTECTION protection_kernel_read;
extern VM_PROTECTION protection_user_write;
extern VM_PROTECTION protection_user_read;
extern VM_PROTECTION protection_all_write;
extern VM_PROTECTION protection_all_read;

extern CACHE virtual_map_cache;
extern CACHE vm_descriptor_cache;

#ifdef __cplusplus
    extern "C" {
#endif

void InitVm();

VIRTUAL_MAP_PTR CreateVirtualMap(VADDR start, VADDR end);

void InitVmDescriptor(VM_DESCRIPTOR_PTR descriptor, VIRTUAL_MAP_PTR vmap, VADDR start, VADDR end, VM_UNIT_PTR vm_unit, VM_PROTECTION_PTR protection);
VM_DESCRIPTOR_PTR CreateVmDescriptor(VIRTUAL_MAP_PTR vmap, VADDR start, VADDR end, VM_UNIT_PTR vm_unit, VM_PROTECTION_PTR protection);
VM_DESCRIPTOR_PTR GetVmDescriptor(VIRTUAL_MAP_PTR vmap, VADDR va, UINT32 size);
void * FindFreeVmRange(VIRTUAL_MAP_PTR vmap, VADDR start, UINT32 size, UINT32 option);
void PrintVmDescriptors(VIRTUAL_MAP_PTR vmap);

VM_UNIT_PTR CreateVmUnit(VM_UNIT_TYPE type, VM_UNIT_FLAG flag, UINT32 size);
void SetVmUnitPage(VM_UNIT_PTR unit, VIRTUAL_PAGE_PTR vp, UINT32 vtop_index);

ERROR_CODE AllocateVirtualMemory(VIRTUAL_MAP_PTR vmap, VADDR * va_ptr, VADDR preferred_start, UINT32 size, UINT32 protection, UINT32 flags, VM_UNIT_PTR unit);
ERROR_CODE FreeVirtualMemory(VIRTUAL_MAP_PTR vmap, VADDR va, UINT32 size, UINT32 flags);
ERROR_CODE MapViewOfFile(int file_id, VADDR * va, UINT32 protection, UINT32 file_offset, UINT32 size, UINT32 preferred_start, UINT32 flags);

ERROR_CODE CopyVirtualAddressRange(VIRTUAL_MAP_PTR src_vmap, VADDR src_va, UINT32 src_size, VIRTUAL_MAP_PTR dest_vmap, VADDR *dest_preferred_va, UINT32 dest_size, UINT32 protection);

VADDR MapPhysicalMemory(VIRTUAL_MAP_PTR vmap, UINT32 pa, UINT32 size, VADDR preferred_va, UINT32 protection);

ERROR_CODE CopyFromUserSpace(void * user_va, void * kernel_va, size_t length);
ERROR_CODE CopyToUserSpace(void * user_va, void * kernel_va, size_t length);

VIRTUAL_MAP_PTR GetCurrentVirtualMap();
ERROR_CODE MemoryFaultHandler(UINT32 va, int is_user_mode, int access_type);

int VirtualMapCacheConstructor(void * buffer);
int VirtualMapCacheDestructor(void * buffer);
int VmDescriptorCacheConstructor(void * buffer);
int VmDescriptorCacheDestructor(void * buffer);

#ifdef __cplusplus
	}
#endif

#endif

