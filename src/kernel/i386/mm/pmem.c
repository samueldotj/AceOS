/*!
	\file		src/kernel/i386/mm/pmem.c	
	\author		Dilip & Samuel
	\version 	3.0
	\date	
  			Created: 24-May-2008 14:37
  			Last modified: Tue May 27, 2008  11:44AM
	\brief	physical memory manager
*/
#include <ace.h>
#include <kernel/error.h>
#include <kernel/multiboot.h>
#include <kernel/debug.h>
#include <kernel/mm/vm.h>
#include <kernel/mm/virtual_page.h>
#include <kernel/mm/pmem.h>
#include <kernel/i386/pmem.h>

PHYSICAL_MAP kernel_physical_map;

MEMORY_AREA	memory_areas[MAX_MEMORY_AREAS];
int memory_area_count;

/*i386 arch specific kernel page directory*/
PAGE_DIRECTORY_ENTRY kernel_page_directory[PAGE_DIRECTORY_ENTRIES] __attribute__ ((aligned (PAGE_SIZE)));

PHYSICAL_MAP kernel_physical_map;

ERROR_CODE EnterPhysicalMapping(PHYSICAL_MAP_PTR pmap, UINT32 va, UINT32 pa, UINT32 protection)
{
	return ERROR_NOT_SUPPORTED;
}
