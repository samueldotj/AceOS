/*!
	\file		src/include/kernel/mm/virtual_page.c	
	\author	Dilip & Samuel
	\version 	3.0
	\date	
  			Created: 24-May-2008 14:37
  			Last modified: 24-May-2008 14:37
	\brief	virtual page
*/

#ifndef __VIRTUAL_PAGE_H
#define __VIRTUAL_PAGE__H

#include <ace.h>
#include <list.h>

struct virtual_page
{
	LIST	page_list;
	LIST	object_list;
};

typedef struct virtual_page VIRTUAL_PAGE, VIRTUAL_PAGE_PTR;

VIRTUAL_PAGE_PTR AllocateVirtualPage();
int FreeVirtualPage(VIRTUAL_PAGE_PTR);

#endif