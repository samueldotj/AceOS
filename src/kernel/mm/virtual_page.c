/*!
	\file		src/kernel/mm/virtual_page.c	
	\author		Samuel
	\version 	3.0
	\date	
  			Created: 02-Jun-2008 10:22pm
  			Last modified: 02-Jun-2008 10:22pm
	\brief	virtual page related routines
*/
#include <kernel/mm/vm.h>
#include <kernel/mm/virtual_page.h>
#include <kernel/debug.h>
#include <string.h>

static void InitVirtualPage(VIRTUAL_PAGE_PTR vp, UINT32 physical_address);

void InitVirtualPageArray(VIRTUAL_PAGE_PTR vpa, UINT32 page_count, UINT32 start_physical_address)
{
	int i;
	for(i=0; i<page_count ;i++)
	{
		InitVirtualPage( &vpa[i], start_physical_address );
		start_physical_address += PAGE_SIZE;
	}
}
static void InitVirtualPage(VIRTUAL_PAGE_PTR vp, UINT32 physical_address)
{
	memset(vp, 0, sizeof( VIRTUAL_PAGE ) );
	
	InitSpinLock( &vp->lock );
	InitList( &vp->page_list );
	InitList( &vp->lru_list );
	vp->free = 1;
	vp->physical_address = physical_address;
}
