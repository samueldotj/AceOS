#define MAX_NUMBER 		100
#define MAX_TREE_NUMBERS 20
#define MAX_DEL_NUMBERS  10

#include <avl_tree.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int random_number_test=0;
int use_predefined_numbers=0;
int use_ascending_order=0;


int i, numbers[MAX_TREE_NUMBERS]={31,15,29,34,27,2,25,6,7,8,9,99,85,64,23,24,1,44,77,71};
int del_numbers[MAX_DEL_NUMBERS]={150, 8, 2, 9, 1, 6}, del_number_index;
	
static void print_usage(char * exe)
{
	printf("Usage : %s /r /p /a /d\n", exe);
	printf("\t /r - Random number test\n");
	printf("\t /p - Predefined number test\n");
	printf("\t /a - Ascending order test\n");
	printf("\t /d - Descending order test\n");
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

int * init_numbers(int * total_numbers, int ** del_numbers_ptr, int * total_del_numbers)
{
	//initialize the numbers
	if ( random_number_test )
	{
		//fill in ascending order
		for(i=0;i<MAX_TREE_NUMBERS;i++)
			numbers[i] = i;
		srand ( time(NULL) );
		//randomize
		for(i=0;i<MAX_TREE_NUMBERS;i++)
		{
			int rand_number = rand()%MAX_NUMBER;
			rand_number %= MAX_TREE_NUMBERS;
			swap(&numbers[i] , &numbers[rand_number] );
		}

		del_number_index=0;
		for(i=0;i<MAX_TREE_NUMBERS;i++)
			if ( !(i%3) && del_number_index<MAX_DEL_NUMBERS)
			{
				del_numbers[del_number_index] = numbers[i];
				del_number_index++;
			}
	}
	else
	{
		if ( !use_predefined_numbers )
		{
			if ( use_ascending_order )
			{
				for(i=0;i<MAX_TREE_NUMBERS;i++)
					numbers[i] = i+1;
			}
			else
			{
				for(i=MAX_TREE_NUMBERS-1;i>=0;i--)
					numbers[i] = MAX_TREE_NUMBERS-i;
			}
		}
		del_number_index = 6;
	}
	
	* total_numbers = MAX_TREE_NUMBERS;
	* del_numbers_ptr = del_numbers;
	* total_del_numbers = del_number_index;
	
	return numbers;
}

void _assert(const char *msg, const char *file, int line)
{
	printf("%s : %s %d", msg, file, line);
	exit(1);
}

