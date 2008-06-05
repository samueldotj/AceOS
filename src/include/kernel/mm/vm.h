/*!
  \file	vm.h
  \author	Samuel (samueldotj@gmail.com)
  \version 	3.0
  \date	
  			Created: 21-Mar-2008 5:13PM
  			Last modified: 
  \brief	virtual memory macros and functions
*/
#ifndef __VM__H
#define __VM__H

#include <ace.h>
#include <ds/list.h>
#include <sync/spinlock.h>
#include <kernel/mm/virtual_page.h>

#if	ARCH == i386
	#define PAGE_SIZE	4096
	#define PAGE_SHIFT	12
#endif

#define PAGE_ALIGN(addr)			((UINT32)(addr) & -PAGE_SIZE)
#define PAGE_ALIGN_UP(addr)			((UINT32)((addr) + PAGE_SIZE - 1) & -PAGE_SIZE)

#define PAGE_ALIGN_4MB(addr)		((UINT32)(addr) & -(4096*1024))
#define PAGE_ALIGN_UP_4MB(addr)		PAGE_ALIGN_4MB( (addr) + (1024*1024) - 1 )

#define NUMBER_OF_PAGES(size)		(PAGE_ALIGN_UP(size) >> PAGE_SHIFT)

typedef struct vm_data
{
	SPIN_LOCK			lock;				//lock for entire structure

	VIRTUAL_PAGE_PTR	free_page_head;		//points to the first free page
	VIRTUAL_PAGE_PTR	inuse_page_head;	//points to the first in use page

}VM_DATA, * VM_DATA_PTR;

extern VM_DATA vm_data;
#ifdef __cplusplus
    extern "C" {
#endif

void InitVm();

#ifdef __cplusplus
	}
#endif

#endif

