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

static void InitVirtualPage(VIRTUAL_PAGE_PTR vp);

void InitVirtualPageArray(VIRTUAL_PAGE_PTR vpa, UINT32 page_count)
{
	int i;
	for(i=0; i<page_count ;i++)
	{
		InitVirtualPage( &vpa[i] );
	}
	kprintf("Init vpa %p count %d \n", vpa, page_count );
}
void InitVirtualPage(VIRTUAL_PAGE_PTR vp)
{
	
}
