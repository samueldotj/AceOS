#define MAX_NUMBER 			1000
#define MAX_TREE_NUMBERS 	500
#define MAX_DEL_NUMBERS  	500

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int rand();
void srand(unsigned int seed);
void exit(int status);

int random_number_test=0;
int use_predefined_numbers=0;
int use_ascending_order=0;
int full_delete=0;
int verbose_level=1;

int i, numbers[MAX_TREE_NUMBERS]={31,15,29,34,27,2,25,6,7,8,9,99,85,64,23,24,1,44,77,71};
int del_numbers[MAX_DEL_NUMBERS]={150, 8, 2, 9, 1, 6};
int max_del_numbers = 6;
int max_tree_numbers = 20;
	
static void print_usage(char * exe)
{
	printf("Usage : %s /r /p /a /d\n", exe);
	printf("\t /r 		- Random number test\n");
	printf("\t /p 		- Predefined number test\n");
	printf("\t /a 		- Ascending order test\n");
	printf("\t /d 		- Descending order test\n");
	printf("\t /f 		- full delete test\n");
	printf("\t /v<n>	- verbose level(0,1,2,3)\n");
	printf("\t /? - shows this message\n");
	printf("\t Note: - can be used instead of / \n");
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
				switch( argv[i][1] )
				{
					case 'r':
						random_number_test = 1;
						break;
					case 'p':
						use_predefined_numbers = 1;
						break;
					case 'a':
						use_ascending_order = 1;
						break;
					case 'd':
						use_ascending_order = 0;
						break;
					case 'f':
						full_delete = 1;
						break;
					case 'v':
						verbose_level = argv[i][2]-'0';
						break;
					default:
						print_usage( argv[0] );
						return 1;
				}
			}
			else
			{
				print_usage( argv[0] );
				return 1;
			}
			i++;
		}
	}
	return 0;
}
static void swap(int * a, int * b)
{
	int tmp = *a;
	*a = *b;
	*b = tmp;
}
static void fill_random_numbers(int * number_array, int capacity, int max_number, int allow_duplicates)
{
	int i;

	for(i=0;i<MAX_TREE_NUMBERS;i++)
	{
		if ( allow_duplicates )
			number_array[i] = rand()%(max_number/2);
		else
			number_array[i] = i+1;
	}
	
	//randomize
	for(i=0;i<MAX_TREE_NUMBERS;i++)
	{
		int rand_number = (rand()%max_number) % capacity;
		swap(&number_array[i] , &number_array[rand_number] );
	}

}
static void select_random_numbers(int * number_array, int * base_array, int capacity, int base_capacity, int allow_duplicates)
{
	int i;
	//fill unique numbers
	for(i=0; i<base_capacity;i++)
		number_array[i] = i;
	//randomize it
	for(i=0;i<base_capacity;i++)
	{
		int rand_number = rand()%base_capacity;
		swap(&number_array[i] , &number_array[rand_number] );
	}
	//select numbers from the base array
	for(i=0;i<capacity && i<base_capacity;i++)
		number_array[i] = base_array[number_array[i]];
}
int * init_numbers(int * total_numbers, int ** del_numbers_ptr, int * total_del_numbers, int allow_duplicates)
{
	if ( !use_predefined_numbers )
	{
		max_tree_numbers = MAX_TREE_NUMBERS;
		if ( full_delete )
			max_del_numbers = MAX_DEL_NUMBERS;
	}

	srand ( time(NULL) );		
	//initialize the numbers
	if ( random_number_test )
		fill_random_numbers( numbers, max_tree_numbers, MAX_NUMBER, allow_duplicates);
	else
	{
		if ( !use_predefined_numbers )
		{
			if ( use_ascending_order )
			{
				for(i=0;i<max_tree_numbers;i++)
					numbers[i] = i+1;
			}
			else
			{
				for(i=max_tree_numbers-1;i>=0;i--)
					numbers[i] = max_tree_numbers-i+1;
			}
		}
	}
	//initialize delete numbers
	if ( !use_predefined_numbers )
		select_random_numbers( del_numbers, numbers, max_del_numbers, MAX_TREE_NUMBERS, allow_duplicates);

	* total_numbers = max_tree_numbers;
	* del_numbers_ptr = del_numbers;
	* total_del_numbers = max_del_numbers;
	
	if ( verbose_level >= 1 )
	{
		printf("       Numbers : " );
		for(i=0;i<max_tree_numbers; i++)
			printf("%2d ", numbers[i]);
		printf("\nDelete numbers : " );
		for(i=0;i<max_del_numbers; i++)
			printf("%2d ", del_numbers[i]);
		printf("\n");
	}
	return numbers;
}

void _assert(const char *msg, const char *file, int line)
{
	printf("%s : %s %d", msg, file, line);
	exit(1);
}


