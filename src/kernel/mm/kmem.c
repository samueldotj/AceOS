/*!
  \file	kernel/mm/kmem.c
  \author	Samuel
  \version 	3.0
  \date	
  			Created: 07-Jul-2008 9:16PM
  			Last modified: 07-Jul-2008 9:16PM
  \brief		
*/
#include <kernel/mm/kmem.h>
#include <kernel/debug.h>

static void * kmem_page_alloc(int size);
static int kmem_page_free(void * va, int size);
static int kmem_page_protect(void * va, int size, int protection);

void InitKmem()
{
	int result;
		
	result = InitHeap(PAGE_SIZE, kmem_page_alloc, kmem_page_free, kmem_page_protect);
	if ( result )
	{
		panic("InitKmem() - InitHeap failed\n");
	}
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



