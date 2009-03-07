/*!
	\file	kernel/mm/pmem.h	
	\brief	physical memory manager
*/

#ifndef __PMEM__H
#define __PMEM__H

#include <ace.h>
#include <kernel/multiboot.h>
#include <kernel/error.h>
#include <kernel/mm/vm_types.h>

/*! Maximum NUMA supported by Ace*/
#define MAX_MEMORY_AREAS		32
/*! Maximum physical regions per "memory areas"*/
#define MAX_PHYSICAL_REGIONS	16

/*http://www.ctyme.com/intr/rb-1741.htm other  not defined yet -- treat as Reserved*/
/*! physical memory available to OS*/
#define PMEM_TYPE_AVAILABLE			0x1
/*! physical memory reserved and not available to OS(e.g. system ROM, memory-mapped device)*/
#define PMEM_TYPE_RESERVED			0x2
/*! physical memory available to OS after reading ACPI tables*/
#define PMEM_TYPE_ACPI_RECLAIM		0x3
/*! ACPI NVS Memory (OS is required to save this memory between NVS*/
#define PMEM_TYPE_ACPI_NVS			0x4


/*! defines a physical memory region inside a "memory area"*/
typedef struct physical_memory_region
{
	UINT32				start_physical_address;			/*! starting physical address of the memory region*/
	UINT32				end_physical_address;			/*! ending physical address of this memory region*/
	
	VIRTUAL_PAGE_PTR	virtual_page_array;				/*! virtual address of virutal page array for this region*/
	UINT32				virtual_page_count;				/*! total virtual pages in this region*/
	
	UINT32				type;							/*! type of this region - usable, reserved, etc*/
	
}PHYSICAL_MEMORY_REGION, * PHYSICAL_MEMORY_REGION_PTR;

/*! defines a "memory area" which contains one or more physical memory regions - NUMA nodes*/
typedef struct memory_area
{
	int						physical_memory_regions_count;		//total regions
	PHYSICAL_MEMORY_REGION 	physical_memory_regions[MAX_PHYSICAL_REGIONS];
}MEMORY_AREA, * MEMORY_AREA_PTR;

typedef enum
{
	VA_NOT_EXISTS=0,
	VA_READABLE,
	VA_WRITEABLE
}VA_STATUS;

extern PHYSICAL_MAP kernel_physical_map;;

extern MEMORY_AREA memory_areas[MAX_MEMORY_AREAS];
extern int memory_area_count;

PHYSICAL_MAP_PTR CreatePhysicalMap(VIRTUAL_MAP_PTR vmap);

void InitPhysicalMemoryManagerPhaseI(unsigned long magic, MULTIBOOT_INFO_PTR mbi);
void InitPhysicalMemoryManagerPhaseII();
void CompletePhysicalMemoryManagerInit();

ERROR_CODE CreatePhysicalMapping(PHYSICAL_MAP_PTR pmap, UINT32 va, UINT32 pa, UINT32 protection);
ERROR_CODE RemovePhysicalMapping(PHYSICAL_MAP_PTR pmap, UINT32 va);

ERROR_CODE MapVirtualAddressRange(PHYSICAL_MAP_PTR pmap, UINT32 va, UINT32 size, UINT32 protection);

VA_STATUS GetVirtualRangeStatus(VADDR va, UINT32 size);
VA_STATUS TranslatePaFromVa(VADDR va, VADDR * pa);

#endif
