/*!
	\file		src/include/kernel/mm/pmem.c	
	\author	Dilip & Samuel
	\version 	3.0
	\date	
  			Created: 24-May-2008 14:37
  			Last modified: Tue May 27, 2008  11:07AM
	\brief	physical memory manager
*/

#include <ace.h>
#include <kernel/mm/virtual_page.h>

#ifndef __PMEM__H
#define __PMEM__H

typedef struct physical_memory_region
{
	UINT32	start_physical_address;			//starting physical address
	UINT32	end_physical_address;			//ending physical address
	UINT32	type;							
	
	VIRTUAL_PAGE_PTR	virtual_page_array;	//virutal page array for this region
	
}PHYSICAL_MEMORY_REGION, * PHYSICAL_MEMORY_REGION_PTR;

typedef struct memory_region
{
	int	count;									//total regions
	PHYSICAL_MEMORY_REGION physical_memory_regions[0];
}MEMORY_REGION, * MEMORY_REGION_PTR;

void InitPhysicalMemory(MEMORY_MAP_PTR memory_map_array, int memory_map_array_size);

#endif
