/*!
  \file         testslab.c
  \author       DilipSimha N M
  \version      3.0
  \date
 			Created:
 			Last modified: Mon Apr 14, 2008  11:38PM
  \brief
*/


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <ace.h>
#include <heap/slab_allocator.h>
#include <sys/mman.h>

#define PAGE_SIZE	4096

extern int verbose_level;

int parse_arguments(int argc, char * argv[]);

void * virtual_alloc(int size);
void * virtual_free(void * va, int size);
void * virtual_protect(void * va, int size, int protection);
int cache1_constructor( void *buffer);
int cache1_destructor( void *buffer);
void AllocateMemory(CACHE_PTR c, void * va_array[], int count);
void FreeMemory(CACHE_PTR c, void * va_array[], int count);


int main(int argc, char * argv[])
{
	int cache_size;
	VADDR va_array[10];

	CACHE cache1;

	if ( parse_arguments(argc, argv) )
		return;
	
	/*intialize slab alloctor*/
	InitSlabAllocator(PAGE_SIZE, virtual_alloc, virtual_free, virtual_protect );
	
	/*intialize  caches*/
	cache_size = 256;
	InitCache(&cache1, 256, 6, 8, 16, &cache1_constructor, &cache1_destructor);
	
	/*FIFO - test*/
	/*allocate memory*/
	AllocateMemory(&cache1, (void**)(&va_array), 10);
	
	/*free memory*/
	FreeMemory(&cache1, (void**)(&va_array), 10);
	
	/*LIFO - test*/
	
	/*random - test*/
}

/*allocate memory from slab and store the va in va_array*/
void AllocateMemory(CACHE_PTR c, void * va_array[], int count)
{
	int i;
	for(i=0;i<count; i++)
	{
		void* va = GetVAFromCache(c, 0);
		if ( va != NULL )
			va_array[i] = va;
	}
}

/*allocate memory from slab and store the va in va_array*/
void FreeMemory(CACHE_PTR c, void * va_array[], int count)
{
	int i;
	for(i=0;i<count; i++)
	{
		FreeBuffer(va_array[i], c);
	}
}

void * virtual_alloc(int size)
{
	/*size should be greater than 0 and should be a multiple of page size*/
	if ( size <= 0 || size % PAGE_SIZE )
	{
		printf("size is incorrect %d\n", size);
		exit(1);
	}
		
	return mmap( 0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, 0, 0);
}

void * virtual_free(void * va, int size)
{
	/*va should be greater than 0 and should be a multiple of page size*/
	if ( va <= 0 || ((unsigned long)va) % PAGE_SIZE)
	{
		printf("va is incorrect %p\n", va);
		exit(1);
	}
	/*size should be greater than 0 and should be a multiple of page size*/
	if ( size <= 0 || size % PAGE_SIZE )
	{
		printf("size is incorrect %d\n", size);
		exit(1);
	}
	munmap(va, size);
}

void * virtual_protect(void * va, int size, int protection)
{
	/*va should be greater than 0 and shoul be in page size*/
	if ( va <= 0 || ((unsigned long)va) % PAGE_SIZE)
	{
		printf("va is incorrect %p\n", va);
		exit(1);
	}
	/*size should be greater than 0 and shoul be in page size*/
	if ( size <= 0 || size % PAGE_SIZE )
	{
		printf("size is incorrect %d\n", size);
		exit(1);
	}
	mprotect( va, size, protection );
}

int cache1_constructor( void *buffer)
{
	
}

int cache1_destructor( void *buffer)
{
	
}
