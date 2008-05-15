#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TEST_TYPE_FIFO			1
#define TEST_TYPE_LIFO			2
#define TEST_TYPE_FREE_RAND		4
#define TEST_TYPE_ALL_RAND		8

int verbose_level=0;
int print_stat = 0;

int alloc_count, cache_size, min_slabs, free_slabs_threshold, max_slabs;

int test_type = 0;

int rand();
void srand(unsigned int seed);
void exit(int status);

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
				printf("Not valid usage\n");
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

