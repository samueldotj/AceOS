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

struct virtual_page
{
	SPIN_LOCK		lock;
	
	LIST			page_list;			/*Resident/free list*/
	LIST			lru_list;			/*LRU List - active/inactive link*/

	UINT32			state;				/*State of the page*/
	
	UINT32 			wire_count;			/*Total wire count*/
	
	UINT32			copy_on_write;		/*No of processes sharing this page*/

	UINT32		
					free:1,				/*if set page is free list*/
					active:1,			/*if set page is in active LRU list*/
					bad:1,				/*if set page is bad*/
					busy:1,				/*if set page is busy due to IO*/
					error:1,			/*if set a page error occurred during last IO*/
					reserved;

	VA_MAP_PTR		va_map_list;		/*list of VAs associated with this page*/

	UINT32 			physical_address;	/*Physical address this page managing*/
};

struct va_map
{
	UINT32				va;					//virtual address for the mapping
	PHYSICAL_MAP_PTR	physical_map;		//pointer to the physical map for va
	
	LIST				list;				//list of all va_map for the virtual page
};

void InitVirtualPageArray(VIRTUAL_PAGE_PTR vpa, UINT32 page_count, UINT32 start_physical_address);

VIRTUAL_PAGE_PTR AllocateVirtualPage();
UINT32 FreeVirtualPage(VIRTUAL_PAGE_PTR vp);

#endif
