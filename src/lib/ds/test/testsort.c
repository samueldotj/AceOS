#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ds/sort.h>

int rand();
void srand(unsigned int seed);
void exit(int status);

#define NUM_ITEMS 100

typedef struct test_item
{
	char test[2];
	int data;
}TEST_ITEM, *TEST_ITEM_PTR;

TEST_ITEM test_items[NUM_ITEMS];

COMPARISION_RESULT compare_data(char * data1, char * data2)
{
	TEST_ITEM_PTR i,j;
	i = (TEST_ITEM_PTR) data1;
	j = (TEST_ITEM_PTR) data2;
	if ( i->data > j->data )
		return GREATER_THAN;
	else if ( i->data < j->data )
		return LESS_THAN;
	return EQUAL;
}
static void swap_test_item(int pos1, int pos2)
{
	TEST_ITEM temp;
	memmove( &temp, &test_items[pos1], sizeof(TEST_ITEM) );
	memmove( &test_items[pos1], &test_items[pos2], sizeof(TEST_ITEM) );
	memmove( &test_items[pos2], &temp, sizeof(TEST_ITEM) );
}
int main()
{
	int i;
	TEST_ITEM temp;
	
	srand( time(NULL) );
	//fill array with random integers
	for(i=0;i<NUM_ITEMS;i++)
		test_items[i].data = i;
	for(i=0;i<NUM_ITEMS;i++)
	{
		int rand_number = rand()%NUM_ITEMS;
		swap_test_item(i, rand_number);
		printf("%i \t", rand_number );
	}
	
	//perform heap sort on array
	SortArray((char *)test_items, (char *)&temp, sizeof(TEST_ITEM), sizeof(test_items)/sizeof(TEST_ITEM), compare_data);

	printf("\n Sort result : \n");

	for (i = 0; i < NUM_ITEMS; i++)
		printf("%i\t", test_items[i].data);
		
	return 0;
}


