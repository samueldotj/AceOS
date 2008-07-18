/*!
  \file		src/lib/include/kernel/mm/kmem.h
  \author	Samuel
  \version 	3.0
  \date	
  			Created: 07-Jul-2008 9:30PM
  			Last modified: 07-Jul-2008 9:30PM
  \brief	
*/

#ifndef _KMEM_H_
#define _KMEM_H_

#include <ace.h>
#include <heap/heap.h>
#include <heap/slab_allocator.h>
#include <kernel/mm/vm.h>

#define	KMEM_DEBUG

#define	KMEM_NO_FAIL	1
#define	KMEM_NO_SLEEP	2

#define	KMEM_RESERVED_MEM_PERCENTAGE	5
	
extern UINT32 kmem_reserved_mem_size;

void InitKmem(VADDR kmem_start_va);

void * kmalloc(int size, UINT32 flag);
int kfree(void * buffer);

#endif
