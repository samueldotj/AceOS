/*!
  \file	vm.h
  \author	Samuel (samueldotj@gmail.com)
  \version 	3.0
  \date	
  			Created: 21-Mar-2008 5:13PM
  			Last modified: 
  \brief	virtual memory macros and functions
*/
#include <ace.h>

#ifndef __VM__H
#define __VM__H

#if	ARCH == i386
	#define PAGE_SIZE	4096
	#define PAGE_SHIFT	12
#endif

#define PAGE_ALIGN(addr)			((UINT32)(addr) & -PAGE_SIZE)
#define PAGE_ALIGN_UP(addr)			((UINT32)((addr) + PAGE_SIZE - 1) & -PAGE_SIZE)

#define PAGE_ALIGN_4MB(addr)		((UINT32)(addr) & -(4096*1024))
#define PAGE_ALIGN_UP_4MB(addr)		PAGE_ALIGN_4MB( (addr) + (1024*1024) - 1 )

#define NUMBER_OF_PAGES(size)		(PAGE_ALIGN_UP(size) >> PAGE_SHIFT)

#ifdef __cplusplus
    extern "C" {
#endif

#ifdef __cplusplus
	}
#endif

#endif

