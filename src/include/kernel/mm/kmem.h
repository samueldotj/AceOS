/*!
  \file		kernel/mm/kmem.h
  \brief	Kernel memory allocation/deallocation routines
*/

#ifndef _KMEM_H_
#define _KMEM_H_

#include <ace.h>
#include <heap/heap.h>
#include <heap/slab_allocator.h>
#include <kernel/mm/vm.h>

/*define this to enable debugging on kmem code*/
#define	KMEM_DEBUG

/*! if this flag is passed, kernel memory allocation should not fail otherwise panic()*/
#define	KMEM_NO_FAIL	1
/*! if this flag is passed, kernel memory allocator will try allocate only from the local pool(not calls VM) otherwise fail*/
#define	KMEM_NO_SLEEP	2

/*kmem reserved memory percentage*/
#define	KMEM_RESERVED_MEM_PERCENTAGE	5
	
extern UINT32 kmem_reserved_mem_size;

void InitKmem(VADDR kmem_start_va);

void * kmalloc(int size, UINT32 flag);
int kfree(void * buffer);

#endif
