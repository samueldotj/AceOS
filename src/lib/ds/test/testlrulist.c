#include <ace.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <malloc.h>
#include <ds/lrulist.h>

void exit(int status);

LRU_LIST lru;
struct ListTest
{
	int data;
	LIST list;
};
typedef struct ListTest LIST_TEST, * LIST_TEST_PTR;

void SpinLockTimeout(SPIN_LOCK_PTR pLockData, void * caller)
{
	printf("spinlock timeout");
	assert(1);
}

LIST_PTR AllocateNode()
{
	LIST_TEST_PTR p;
	p =(LIST_TEST_PTR) malloc(sizeof(LIST_TEST));
	if( p==NULL )
	{
		perror("LIST_TEST allocation failed");
		return 0;
	}
	return &p->list;
}
void FreeNode(LIST_PTR node)
{
	LIST_TEST_PTR p;
	assert( node != NULL );
	p = STRUCT_ADDRESS_FROM_MEMBER(node, LIST_TEST, list);
	free(p);
}
int main(int argc, char* argv[])
{
	int i=0;
	LIST_PTR n, ref=NULL, del=NULL;
	
	InitLruList( &lru, 10, AllocateNode, NULL, FreeNode );
	
	printf("LRU List Test Program\n");

	/*insert into list*/
	printf("Inserting numbers 0 to 20\n");
	for(i=1;i<20;i++)
	{
		LIST_PTR p = AllocateLruNode(&lru);
		STRUCT_ADDRESS_FROM_MEMBER(p, LIST_TEST, list)->data = i;
		if ( i == 4 )
			ref = p;
		if ( i == 18 )
			del = p;
		if ( ref )
			ReferenceLruNode( &lru, ref);
			
		//printf("lru->current_count %d %p %p\n", lru.current_count, lru.list_head, p);
	}
	FreeLruNode( &lru, del );
	printf("printing list..\n");
	i=0;
	LIST_FOR_EACH(n, lru.list_head)
	{
		printf("%d) Node = %d\n", i++, STRUCT_ADDRESS_FROM_MEMBER(n, LIST_TEST, list)->data );
	}

	return 0;
}

