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

typedef struct virtual_page
{
	LIST	page_list;
	LIST	object_list;
}VIRTUAL_PAGE, * VIRTUAL_PAGE_PTR;

void InitVirtualPageArray(VIRTUAL_PAGE_PTR vpa, UINT32 page_count);

VIRTUAL_PAGE_PTR AllocateVirtualPage();
int FreeVirtualPage(VIRTUAL_PAGE_PTR);

#endif
