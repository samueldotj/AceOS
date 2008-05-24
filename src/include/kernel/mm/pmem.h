/*!
	\file		src/include/kernel/mm/pmem.c	
	\author	Dilip & Samuel
	\version 	3.0
	\date	
  			Created: 24-May-2008 14:37
  			Last modified: 24-May-2008 14:37
	\brief	physical memory manager
*/

#include <ace.h>

#ifndef __PMEM__H
#define __PMEM__H

typedef struct physical_memory_region
{
	UINT32	start_physical_address;			//starting physical address
	UINT32	end_physical_address;			//ending physical address
	UINT32	type;							
	
	VIRTUAL_PAGE_PTR	virtua_page_array;	//virutal page array for this region
	
}PHYSICAL_MEMORY_REGION, * PHYSICAL_MEMORY_REGION_PTR;

typedef struct memory_regions
{
	int	count;									//total regions
	PHYSICAL_MEMORY_REGION physical_memory_regions[0];
}MEMORY_REGIONS, * MEMORY_REGIONS_PTR;

int InitPhysicalMemory(struct multiboot_info * mb);

#endif