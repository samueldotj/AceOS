/*!
  \file         testslab.c
  \author       DilipSimha N M
  \version      3.0
  \date
 			Created:
 			Last modified: Fri May 02, 2008  02:01PM
  \brief
*/
#include <sys/mman.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ace.h>
#include <heap/slab_allocator.h>
#include "leak_detector_c.h"

#define PAGE_SIZE	4096

#define TEST_TYPE_FIFO			1
#define TEST_TYPE_LIFO			2
#define TEST_TYPE_FREE_RAND		4
#define TEST_TYPE_ALL_RAND		8

extern int verbose_level;
extern int test_type;

int parse_arguments(int argc, char * argv[]);
void fill_random_numbers(int * number_array, int capacity, int max_number);
int rand();
void srand(unsigned int seed);
void exit(int status);

void free(void *);

void * virtual_alloc(int size);
int virtual_free(void * va, int size);
int virtual_protect(void * va, int size, int protection);
int cache_constructor( void *buffer);
int cache_destructor( void *buffer);
void AllocateMemory(CACHE_PTR c, void * va_array[], int count);
void FreeMemoryFifo(CACHE_PTR c, void * va_array[], int count);
void FreeMemoryLifo(CACHE_PTR c, void * va_array[], int count);
void FreeMemoryRandom(CACHE_PTR c, void * va_array[], int count);
void RandomMemoryAllocFree(CACHE_PTR c, void * va_array[], int array_size, int min_run);

extern int alloc_count, cache_size, min_slabs, free_slabs_threshold, max_slabs;

#define PRINT(verbose, string)	if( verbose_level >= verbose ) printf(string);

int main(int argc, char * argv[])
{
	VADDR * va_array;
	
	CACHE cache;
	

	if ( parse_arguments(argc, argv) )
		return 1;
	
	srand ( time(NULL) );
	atexit(report_mem_leak);

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
	
	if ( InitCache(&cache, cache_size, free_slabs_threshold, min_slabs, max_slabs, &cache_constructor, &cache_destructor) == -1 )
	{
		printf("Initializing cache failed");
		return 1;
	}
	
	
	//FIFO - test
	if ( test_type & TEST_TYPE_FIFO )
	{
		PRINT( 1, "FIFO Test : allocating memory from cache\n");
		AllocateMemory(&cache, (void **)va_array, alloc_count );
		PRINT( 1, "FIFO Test : free buffer\n");
		FreeMemoryFifo(&cache, (void **)va_array, alloc_count);
	}
	//LIFO - test
	if ( test_type & TEST_TYPE_LIFO )
	{
		PRINT( 1, "LIFO Test : allocating memory from cache\n");
		AllocateMemory(&cache, (void **)va_array, alloc_count );
		PRINT( 1, "LIFO Test : free buffer\n");
		FreeMemoryLifo(&cache, (void **)va_array, alloc_count);
	}
	//random - test
	if ( test_type & TEST_TYPE_FREE_RAND )
	{
		PRINT( 1, "Random Test : allocating memory from cache\n");
		AllocateMemory(&cache, (void **)va_array, alloc_count );
		PRINT( 1, "Random Test : free buffer\n");
		FreeMemoryRandom(&cache, (void **)va_array, alloc_count);
	}

	//completely random
	if ( test_type & TEST_TYPE_ALL_RAND )
	{
		PRINT( 1, "Random Alloc & Free Test : \n");
		RandomMemoryAllocFree(&cache, (void **)va_array, alloc_count, 2+(rand()%20) );
	}
	
	DestroyCache( &cache );
	PRINT(1, "Cache destroyed\n");
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
			printf("Freed buffer %p, %d\n", (VADDR*)(va_array[i]), i);
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
			printf("Freed buffer %p %d\n", (VADDR*)(va_array[i]), i);
		}
	}
}

void FreeMemoryRandom(CACHE_PTR c, void * va_array[], int count)
{
	int i;
	int * rand_array = (int *)calloc(count, sizeof(int));
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
			printf("Freed buffer %p %d\n", (VADDR*)(va_array[j]), i);
		}
	}
	free(rand_array);
}

void RandomMemoryAllocFree(CACHE_PTR c, void * va_array[], int array_size, int min_run)
{
	int i, not_freed=0;
	for(i=0; i<min_run; i++)
	{
		int allocate_count;
		int free_count;
		int j, * free_index_array;
		
		/*randomly select a allocate count*/
		allocate_count = rand() % (array_size-not_freed);
		
		//allocate memory
		AllocateMemory(c, &va_array[not_freed], allocate_count);
		
		/*recalculate not freed count*/
		not_freed+= allocate_count;
		
		/*randomly select a allocate count*/
		free_count = rand() % not_freed;
		
		assert( not_freed >= 0 );
		assert( allocate_count <= array_size );
		assert( not_freed <= array_size );
		assert( free_count <= not_freed );
		
		//prepare random free
		free_index_array = calloc(not_freed, sizeof(int));
		if ( free_index_array == NULL )
		{
			perror("calloc ");
			exit(1);
		}
		fill_random_numbers(free_index_array, not_freed, not_freed );
		
		//free memory based on random index
		for(j=0; j<free_count; j++)
		{
			int free_index = free_index_array[j];
			assert(  free_index < not_freed );
			if ( verbose_level >= 2 ) printf("Freeing VA %p(%d) : ",va_array[free_index], free_index);
			if ( FreeBuffer(va_array[free_index_array[j]], c) == -1 )
			{
				printf("FreeBuffer(%p, %p) %d %d failed\n", va_array[free_index], c, free_index, j);
				exit(1);
			}
			if ( verbose_level >= 2 ) printf("ok\n");
			va_array[free_index] = NULL;
			
		}
		free(free_index_array);
		
		//rearrange(move the freed elements to the end) and resize(decrease not_freed count) the va_array
		for(j=0; j<not_freed ; j++)
		{
			//skip the freed one
			while( j<not_freed && va_array[not_freed] == NULL )
				not_freed--;
			if ( j < not_freed )
			{
				//if va is freed; move last element to this place
				if ( va_array[j] == NULL )
				{
					va_array[j] = va_array[not_freed];
					va_array[not_freed] = 0;
					not_freed--;
				}
			}
		}
		//recalculate the not_freed entries
		for(not_freed=0; va_array[not_freed] != 0 && not_freed<array_size ; not_freed++);
		
		if (verbose_level >=1 ) printf("Pass(%d/%d) Allocated %3d Freed %3d Still in cache %3d\n", i+1, min_run, allocate_count, free_count, not_freed );
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
