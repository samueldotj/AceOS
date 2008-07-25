/*!
  \file	kernel/mm/kmem.c
  \author	Samuel
  \version 	3.0
  \date	
  			Created: 07-Jul-2008 9:16PM
  			Last modified: 07-Jul-2008 9:16PM
  \brief		
*/
#include <ds/align.h>
#include <kernel/debug.h>
#include <kernel/mm/kmem.h>
#include <kernel/mm/pmem.h>

static void * kmem_page_alloc(int size);
static int kmem_page_free(void * va, int size);
static int kmem_page_protect(void * va, int size, int protection);
UINT32 kmem_reserved_mem_size=0;

void InitKmem(VADDR kmem_start_va)
{
	if ( kmem_reserved_mem_size==0 )
		kmem_reserved_mem_size = (KMEM_RESERVED_MEM_PERCENTAGE * vm_data.total_memory_pages) /100;
		
	kmem_reserved_mem_size = PAGE_ALIGN_UP(kmem_reserved_mem_size*PAGE_SIZE);
	
	kprintf("kmem: preallocating memory from 0x%p to 0x%p (%d KB)\n", kmem_start_va, ((char*)kmem_start_va)+kmem_reserved_mem_size, kmem_reserved_mem_size/1024 );
	if ( MapVirtualAddressRange( &kernel_physical_map, kmem_start_va, kmem_reserved_mem_size, PROT_WRITE) != ERROR_SUCCESS )
		panic("InitKmem() - MapVirtualAddressRange() failed.");

	if ( InitHeap(PAGE_SIZE, kmem_page_alloc, kmem_page_free, kmem_page_protect) )
		panic("InitKmem() - InitHeap() failed.");
	
	if ( AddMemoryToHeap((char *)kmem_start_va, ((char*)kmem_start_va)+kmem_reserved_mem_size ) != 0 )
		panic("InitKmem() -  AddMemoryToHeap() failed.");
		
	kernel_free_virtual_address = ((VADDR)kmem_start_va)+kmem_reserved_mem_size;
}
static void * kmem_page_alloc(int size)
{
	VADDR va;
	ERROR_CODE ret;
#ifdef KMEM_DEBUG
	if ( size <= 0 || size % PAGE_SIZE )
		panic("kmem_page_alloc() - size is incorrect\n");
#endif
	ret = AllocateVirtualMemory(&kernel_map, &va, 0, size, 0, 0);
	if ( ret == ERROR_SUCCESS )
		return (void *)va;
	else
		return NULL;
}
static int kmem_page_free(void * va, int size)
{
#ifdef KMEM_DEBUG
	if ( va <= 0 || ((unsigned long)va) % PAGE_SIZE)
		panic("kmem_page_free() - va is incorrect\n");
	/*size should be greater than 0 and should be a multiple of page size*/
	if ( size <= 0 || size % PAGE_SIZE )
		panic("kmem_page_free() - size is incorrect\n");
#endif
	return FreeVirtualMemory(&kernel_map, (VADDR)va, size, NULL);
}
static int kmem_page_protect(void * va, int size, int protection)
{
#ifdef KMEM_DEBUG
	if ( va <= 0 || ((unsigned long)va) % PAGE_SIZE)
		panic("kmem_page_protect() - va is incorrect\n");
	/*size should be greater than 0 and should be a multiple of page size*/
	if ( size <= 0 || size % PAGE_SIZE )
		panic("kmem_page_protect() - size is incorrect\n");
#endif
	return 1;
}

void * kmalloc(int size, UINT32 flag)
{
	void * ret = AllocateFromHeap(size);
	if ( ret==NULL && (flag & KMEM_NO_FAIL) )
		panic("kmalloc failed");
	return ret;
}

int kfree(void * buffer)
{
	return FreeToHeap(buffer);
}



