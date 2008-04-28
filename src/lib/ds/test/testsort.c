#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <ds/sort.h>

int rand();
void srand(unsigned int seed);
void exit(int status);

#define NUM_ITEMS 100

int numbers[NUM_ITEMS];

COMPARISION_RESULT compare_int(char * data1, char * data2)
{
	int i,j;
	i = *((int *) data1);
	j = *((int *) data2);
	if ( i > j )
		return GREATER_THAN;
	else if ( i < j )
		return LESS_THAN;
	return EQUAL;
}

int main()
{
	int i;
	int temp;
	
	srand( time(NULL) );

	//fill array with random integers
	for (i = 0; i < NUM_ITEMS; i++)
	{
		numbers[i] = rand()%NUM_ITEMS;
		printf("%i \t", numbers[i]);
	}	
		
	//perform heap sort on array
	Sort((char *)numbers, (char *)&temp, sizeof(int), sizeof(numbers)/sizeof(int), compare_int);

	printf("\n Sort result : \n");

	for (i = 0; i < NUM_ITEMS; i++)
		printf("%i\t", numbers[i]);
		
	return 0;
}


