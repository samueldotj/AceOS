#include "testcommon.h"

int verbose_level=0;
int print_stat = 0;

int alloc_count, cache_size, min_slabs, free_slabs_threshold, max_slabs;

int test_type = 0;

int rand();
void srand(unsigned int seed);
void exit(int status);
int atoi ( const char * str );

static void print_usage(char * exe)
{
	printf("Usage : %s [/fifo] [/lifo] [/free_random] [/all_random] [/verbose <N>] /cache_size <N> /min_slabs <N> /max_slabs <N> /free_slabs_threshold <N> \n", exe);
}
int parse_arguments(int argc, char * argv[])
{
	//parse arguments
	if ( argc > 1 )
	{
		int i=1;
		while( i < argc )
		{
			if ( argv[i][0] == '/' || argv[i][0] == '-')
			{
				if ( !strcmp( &argv[i][1], "fifo") )
				{
					test_type |= TEST_TYPE_FIFO;
				}
				else if ( !strcmp( &argv[i][1], "lifo") )
				{
					test_type |= TEST_TYPE_LIFO;
				}
				else if ( !strcmp( &argv[i][1], "free_random") )
				{
					test_type |= TEST_TYPE_FREE_RAND;
				}
				else if ( !strcmp( &argv[i][1], "all_random") )
				{
					test_type |= TEST_TYPE_ALL_RAND;
				}
				else if ( !strcmp( &argv[i][1], "verbose") && (i+1) < argc)
				{
					i++;
					verbose_level = atoi( argv[i] );
				}
				else if ( !strcmp( &argv[i][1], "alloc_count") && (i+1) < argc)
				{
					i++;
					alloc_count = atoi( argv[i] );
				}
				else if ( !strcmp( &argv[i][1], "cache_size") && (i+1) < argc)
				{
					i++;
					cache_size = atoi( argv[i] );
				}
				else if ( !strcmp( &argv[i][1], "min_slabs") && (i+1) < argc)
				{
					i++;
					min_slabs = atoi( argv[i] );
				}
				else if ( !strcmp( &argv[i][1], "max_slabs") && (i+1) < argc)
				{
					i++;
					max_slabs = atoi( argv[i] );
				}
				else if ( !strcmp( &argv[i][1], "free_slabs_threshold") && (i+1) < argc)
				{
					i++;
					free_slabs_threshold = atoi( argv[i] );
				}
				else
				{
					printf("Unrecognized option %s\n", argv[i] );
					print_usage( argv[0] );
					return 1;
				}
				
			}
			else
			{
				printf("Not a valid usage\n");
				print_usage( argv[0] );
				return 1;
			}
			i++;
		}
	}
	
	return 0;
}

void _assert(const char *msg, const char *file, int line)
{
	printf("%s : %s %d", msg, file, line);
	exit(1);
}

void swap(int * a, int * b)
{
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

void fill_random_numbers(int * number_array, int capacity, int max_number)
{
	int i;
	//fill in ascending order
	for(i=0;i<capacity;i++)
		number_array[i] = i;
	
	//randomize
	for(i=0;i<capacity;i++)
	{
		int rand_number = (rand()%max_number) % capacity;
		swap(&number_array[i] , &number_array[rand_number] );
	}
}
void print_stats(CACHE_PTR cache_ptr)
{
	CACHE_STATISTICS_PTR stat = GetCacheStatistics(cache_ptr);
	if ( stat == NULL )
	{
		printf("Cache Statistics are not enabled\n");
		return;
	}
	printf("Cache Statistics : \n");
	printf("\t alloc() calls : %d Success : %d Failures : %d \n", (int)stat->alloc_calls, (int)stat->alloc_calls - (int)stat->alloc_failures, (int)stat->alloc_failures);
	printf("\t free() calls: %d\n", (int)stat->free_calls);
	
	printf("\t vm_alloc_calls() : %d vm_free_calls() : %d \n", (int)stat->vm_alloc_calls, (int)stat->vm_free_calls);
	
	printf("\t peak slab usage : %d average usage : %d\n", (int)stat->max_slabs_used, (int)stat->average_slab_usage );
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
void SpinLockTimeout(SPIN_LOCK_PTR pLockData, void * caller)
{
	printf("spinlock timeout");
	exit(1);
}
