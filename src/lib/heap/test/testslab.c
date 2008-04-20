/*!
  \file         testslab.c
  \author       DilipSimha N M
  \version      3.0
  \date
 			Created:
 			Last modified: Sun Apr 20, 2008  02:15AM
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
int cache_constructor( void *buffer);
int cache_destructor( void *buffer);
void AllocateMemory(CACHE_PTR c, void * va_array[], int count);
void FreeMemoryFifo(CACHE_PTR c, void * va_array[], int count);
void FreeMemoryLifo(CACHE_PTR c, void * va_array[], int count);
void FreeMemoryRandom(CACHE_PTR c, void * va_array[], int count);

extern int alloc_count, cache_size, min_slabs, free_slabs_threshold, max_slabs;

#define PRINT(verbose, string)	if( verbose_level >= verbose ) printf(string);

int main(int argc, char * argv[])
{
	CACHE cache;
	VADDR * va_array;

	if ( parse_arguments(argc, argv) )
		return 1;
	
	printf("Slab Allocator Test : alloc_count %d cache_size %d, min_slabs %d, free_slabs_threshold %d, max_slabs %d\n", 
								alloc_count, cache_size, min_slabs, free_slabs_threshold, max_slabs);
	va_array = (VADDR *) calloc(alloc_count, sizeof(VADDR));
	if ( va_array == NULL )
	{
		perror("malloc ");
		return 1;
	}
	/*intialize slab alloctor*/
	InitSlabAllocator(PAGE_SIZE, virtual_alloc, virtual_free, virtual_protect );
	PRINT( 2, "Initialized Slab allocator\n" );
	
	PRINT( 2, "Initializing cache\n");	
	if ( InitCache(&cache, cache_size, free_slabs_threshold, min_slabs, max_slabs, &cache_constructor, &cache_destructor) == -1 )
	{
		printf(" failed");
		return 1;
	}
	
	
	/*FIFO - test*/
	PRINT( 1, "FIFO Test : allocating memory from cache\n");
	AllocateMemory(&cache, (void **)va_array, alloc_count );
	PRINT( 1, "FIFO Test : free buffer\n");
	FreeMemoryFifo(&cache, (void **)va_array, alloc_count);
	
	
	/*LIFO - test*/
	PRINT( 1, "LIFO Test : allocating memory from cache\n");
	AllocateMemory(&cache, (void **)va_array, alloc_count );
	PRINT( 1, "LIFO Test : free buffer\n");
	FreeMemoryLifo(&cache, (void **)va_array, alloc_count);
	
	/*random - test*/
	PRINT( 1, "Random Test : allocating memory from cache\n");
	AllocateMemory(&cache, (void **)va_array, alloc_count );
	PRINT( 1, "Random Test : free buffer\n");
	FreeMemoryLifo(&cache, (void **)va_array, alloc_count);
	
	DestroyCache( &cache );
	PRINT(2, "Cache destroyed\n");
	
	free(va_array);
	
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
void FreeMemoryFifo(CACHE_PTR c, void * va_array[], int count)
{
	int i;
	for(i=0;i<count; i++)
	{
		if ( FreeBuffer(va_array[i], c) == -1 )
		{
			printf("FreeBuffer(%p, %p) %d failed\n", va_array[i], c, i);
			exit(1);
		}
		if (verbose_level >=2)
		{
			printf("Freed buffer %p\n", (VADDR*)(va_array[i]));
		}
	}
}
void FreeMemoryLifo(CACHE_PTR c, void * va_array[], int count)
{
	int i;
	for(i=count-1;i>=0; i--)
	{
		if ( FreeBuffer(va_array[i], c) == -1 )
		{
			printf("FreeBuffer(%p, %p) %d failed\n", va_array[i], c, i);
			exit(1);
		}
		if (verbose_level >=2)
		{
			printf("Freed buffer %p\n", (VADDR*)(va_array[i]));
		}
	}
}
void FreeMemoryRandom(CACHE_PTR c, void * va_array[], int count)
{
	int i;
	int * rand_array = (int *)calloc(count, 1);
	if ( rand_array == NULL )
	{
		perror("calloc ");
		exit(1);
	}
	fill_random_numbers( rand_array, count, count );
	for(i=count-1;i>=0; i--)
	{
		int j = rand_array[i];
		if ( FreeBuffer(va_array[j], c) == -1 )
		{
			printf("FreeBuffer(%p, %p) %d %d failed\n", va_array[j], c, i, j);
			exit(1);
		}
		if (verbose_level >=2)
		{
			printf("Freed buffer %p\n", (VADDR*)(va_array[j]));
		}
	}
	free(rand_array);
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

int cache_constructor( void *buffer)
{
	int i, j;
	char pattern[]="ERROR ";
	//fill the repeating pattern in the buffer
	for(i=0; i<cache_size; i++, j++)
	{
		((char *)buffer)[i] = pattern[j];
		if ( j > sizeof(pattern) )
			j=0;
	}
	return 0;
}

int cache_destructor( void *buffer)
{
	return 0;
}
