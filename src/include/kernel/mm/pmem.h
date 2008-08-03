/*!
	\file		src/include/kernel/mm/pmem.c	
	\author	Dilip & Samuel
	\version 	3.0
	\date	
  			Created: 24-May-2008 14:37
  			Last modified: Tue May 27, 2008  11:07AM
	\brief	physical memory manager
*/

#ifndef __PMEM__H
#define __PMEM__H

#include <ace.h>
#include <kernel/multiboot.h>
#include <kernel/error.h>
#include <kernel/mm/vm_types.h>

#define MAX_MEMORY_AREAS		32
#define MAX_PHYSICAL_REGIONS	16

/*http://www.ctyme.com/intr/rb-1741.htm
Values for System Memory Map address type:
01h    memory, available to OS
02h    reserved, not available (e.g. system ROM, memory-mapped device)
03h    ACPI Reclaim Memory (usable by OS after reading ACPI tables)
04h    ACPI NVS Memory (OS is required to save this memory between NVS
sessions)
other  not defined yet -- treat as Reserved*/
#define PMEM_TYPE_AVAILABLE			0x1
#define PMEM_TYPE_RESERVED			0x2
#define PMEM_TYPE_ACPI_RECLAIM		0x3
#define PMEM_TYPE_ACPI_NVS			0x4


typedef struct physical_memory_region
{
	UINT32				start_physical_address;			//starting physical address
	UINT32				end_physical_address;			//ending physical address
	
	VIRTUAL_PAGE_PTR	virtual_page_array;				//virutal page array for this region
	UINT32				virtual_page_count;				//total pages in this region
	
	UINT32				type;							//usable, reserved, etc
	
}PHYSICAL_MEMORY_REGION, * PHYSICAL_MEMORY_REGION_PTR;

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

void InitPhysicalMemoryManagerPhaseI(unsigned long magic, MULTIBOOT_INFO_PTR mbi);
void InitPhysicalMemoryManagerPhaseII();

ERROR_CODE CreatePhysicalMapping(PHYSICAL_MAP_PTR pmap, UINT32 va, UINT32 pa, UINT32 protection);
ERROR_CODE RemovePhysicalMapping(PHYSICAL_MAP_PTR pmap, UINT32 va);

ERROR_CODE MapVirtualAddressRange(PHYSICAL_MAP_PTR pmap, UINT32 va, UINT32 size, UINT32 protection);

VA_STATUS GetVirtualRangeStatus(VADDR va, UINT32 size);

#endif
