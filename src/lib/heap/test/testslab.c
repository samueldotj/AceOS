/*!
  \file         testslab.c
  \author       DilipSimha N M
  \version      3.0
  \date
 			Created:
 			Last modified: Thu Apr 17, 2008  11:27PM
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
int virtual_free(void * va, int size);
int virtual_protect(void * va, int size, int protection);
int cache1_constructor( void *buffer);
int cache1_destructor( void *buffer);
void AllocateMemory(CACHE_PTR c, void * va_array[], int count);
void FreeMemory(CACHE_PTR c, void * va_array[], int count);


int main(int argc, char * argv[])
{
	int cache_size;
	VADDR va_array[10];
	int total_array_elements = sizeof(va_array) / sizeof(VADDR);

	CACHE cache1;

	if ( parse_arguments(argc, argv) )
		return 1;
	
	/*intialize slab alloctor*/
	InitSlabAllocator(PAGE_SIZE, virtual_alloc, virtual_free, virtual_protect );
	if (verbose_level >= 2)
	{
		printf("Initialized Slab allocator\n");
	}
	
	/*intialize  caches*/
	cache_size = 256;
	InitCache(&cache1, 256, 6, 8, 16, &cache1_constructor, &cache1_destructor);
	if (verbose_level >= 2)
	{
		printf("Initialized cache\n");
	}
	
	/*FIFO - test*/
	/*allocate memory*/
	AllocateMemory(&cache1, (void **)va_array, total_array_elements );
	if (verbose_level >= 2)
	{
		printf("Allocated memory to cache\n");
	}
	
	/*free memory*/
	FreeMemory(&cache1, (void **)va_array, total_array_elements);
	if (verbose_level >= 2)
	{
		printf("iFreed memory from cache\n");
	}
	
	/*LIFO - test*/
	
	/*random - test*/
	
	
	DestroyCache( &cache1 );
	if (verbose_level >= 2)
	{
		printf("Cache destroyed\n");
	}
	
	return 0;
}

/*allocate memory from slab and store the va in va_array*/
void AllocateMemory(CACHE_PTR c, void * va_array[], int count)
{
	int i;
	for(i=0;i<count; i++)
	{
		void* va = GetVAFromCache(c, 0);
		if ( va == NULL )
		{
			printf("GetVAFromCache(%p, 0) failed\n", c);
			exit(1);
		}
		if (verbose_level >=2)
		{
			printf("Allocated memory %p %d\n", va, i);
		}
		va_array[i] = va;
	}
}

/*allocate memory from slab and store the va in va_array*/
void FreeMemory(CACHE_PTR c, void * va_array[], int count)
{
	int i;
	for(i=0;i<count; i++)
	{
		if ( FreeBuffer(va_array[i], c) == -1 )
		{
			printf("FreeBuffer(%p, %p) %d failed\n", va_array[i], c, i);
			exit(1);
		}
	}
}

void * virtual_alloc(int size)
{
	void * va;
	/*size should be greater than 0 and should be a multiple of page size*/
	if ( size <= 0 || size % PAGE_SIZE )
	{
		printf("size is incorrect %d\n", size);
		exit(1);
	}
	if (verbose_level >=2)
	{
		printf("virtual_alloc: size=%d\n", size);
	}

	va = mmap( 0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if ( va == MAP_FAILED )
	{
		perror("mmap ");
		exit(0);
	}
		
	return va;
}

int virtual_free(void * va, int size)
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
	
	if (verbose_level >=2)
	{
		printf("virtual_free: size=%d va=%p\n", size, (VADDR*)(va));
	}

	return munmap(va, size);
}

int virtual_protect(void * va, int size, int protection)
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
	return mprotect( va, size, protection );
}

int cache1_constructor( void *buffer)
{
	return 0;
}

int cache1_destructor( void *buffer)
{
	return 0;
}
