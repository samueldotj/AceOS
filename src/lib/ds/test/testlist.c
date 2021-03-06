#include <ace.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <malloc.h>
#include <ds/list.h>

void exit(int status);

struct ListTest
{
	LIST list;
	int data;
	LIST full_list;
};
typedef struct ListTest LIST_TEST, * LIST_TEST_PTR;

int main(int argc, char* argv[])
{
	int i=0;
	LIST_TEST odd_head, even_head, head;
	odd_head.data = -1;
	even_head.data = -2;
	head.data = 0;

	InitList(&odd_head.list);
	InitList(&even_head.list);

	InitList(&head.full_list);


	printf("Link List Test Program\n");


	/*insert into list*/
	printf("Inserting numbers -2 to 10\n");
	for(i=-2;i<=10;i++)
	{
		LIST_TEST_PTR p;

		p =(LIST_TEST_PTR) malloc(sizeof(LIST_TEST));
		if( p==NULL )
		{
			perror("LIST_TEST allocation failed");
			return 0;
		}
		InitList(&p->list);
		InitList(&p->full_list);

		p->data =i;

		if ( i%2 )
			AddToList(&even_head.list, &p->list );
		else
			AddToList(&odd_head.list, &p->list );

		AddToList(&head.full_list, &p->full_list );
	}


	//print list
	LIST_PTR n, tmp;

	printf("printing odd list..\n");
	i=0;
	LIST_FOR_EACH(n, &odd_head.list)
	{
		printf("%d) Node = %d\n", i++, STRUCT_ADDRESS_FROM_MEMBER(n, LIST_TEST, list)->data );

	}

	printf("printing even list..\n");
	i=0;
	LIST_FOR_EACH(n, &even_head.list)
	{
		printf("%d) Node = %d\n", i++, STRUCT_ADDRESS_FROM_MEMBER(n, LIST_TEST, list)->data );
	}
	
	printf("printing full list..\n");
	i=0;
	LIST_FOR_EACH(n, &head.full_list)
	{
		printf("%d) Node = %d\n", i++, STRUCT_ADDRESS_FROM_MEMBER(n, LIST_TEST, full_list)->data );
	}


	/*remove 5 and 8*/
	printf("removing 5 and 8..\n");
	i=0;
	LIST_FOR_EACH_SAFE(n, tmp, &head.full_list)
	{
		if ( STRUCT_ADDRESS_FROM_MEMBER(n, LIST_TEST, full_list)->data == 5 ||
			 STRUCT_ADDRESS_FROM_MEMBER(n, LIST_TEST, full_list)->data == 8 
			 )
			RemoveFromList(n);
	}
	printf("printing full list again..\n");
	LIST_FOR_EACH(n, &head.full_list)
	{
		printf("%d) Node = %d\n", i++, STRUCT_ADDRESS_FROM_MEMBER(n, LIST_TEST, full_list)->data );
	}

	
	return 0;
}

