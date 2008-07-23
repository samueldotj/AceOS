/*!
	\file		src/include/kernel/mm/virtual_page.c	
	\author	Dilip & Samuel
	\version 	3.0
	\date	
  			Created: 24-May-2008 14:37
  			Last modified: Tue May 27, 2008  11:18AM
	\brief	virtual page
*/

#ifndef __VIRTUAL_PAGE_H
#define __VIRTUAL_PAGE_H

#include <ace.h>
#include <ds/list.h>
#include <sync/spinlock.h>
#include <kernel/mm/vm_types.h>

#define VP_TO_PHYS(vp)		(vp->physical_address)
#define PHYS_TO_VP(phys)	(PhysicalToVirtualPage(phys))

struct virtual_page
{
	SPIN_LOCK	lock;
	
	BYTE
				free:1,				/*if set page is free list*/
				active:1,			/*if set page is in active LRU list*/
				bad:1,				/*if set page is bad*/
				busy:1,				/*if set page is busy due to IO*/
				error:1,			/*if set a page error occurred during last IO*/
				reserved;
	union
	{
		struct
		{
			VIRTUAL_PAGE_PTR	free_first_page;/*pointer to starting page of this free physical range*/
			AVL_TREE_D			free_tree;		/*this is starting of a free physical range*/
			UINT32				free_size;  	/*size of the free range in page units*/
		};
		struct
		{
			LIST				lru_list;			/*LRU List - active/inactive link*/

			UINT16				copy_on_write;		/*No of processes sharing this page*/
			UINT16 				wire_count;			/*Total wire count*/

			VA_MAP_PTR			va_map_list;		/*list of VAs associated with this page*/
		};
	};
	
	UINT32 		physical_address;	/*Physical address this page managing*/
}__attribute__ ((packed));

struct va_map
{
	UINT32				va;					//virtual address for the mapping
	PHYSICAL_MAP_PTR	physical_map;		//pointer to the physical map for va
	
	LIST				list;				//list of all va_map for the virtual page
}__attribute__ ((packed));;

void InitVirtualPageArray(VIRTUAL_PAGE_PTR vpa, UINT32 page_count, UINT32 start_physical_address);

VIRTUAL_PAGE_PTR AllocateVirtualPages(int pages);
UINT32 FreeVirtualPages(VIRTUAL_PAGE_PTR vp, int pages);

VIRTUAL_PAGE_PTR PhysicalToVirtualPage(UINT32 physical_address);

#endif
