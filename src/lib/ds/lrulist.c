/*!
	\file		lrulist.c
	\brief	Least recently used list
	
	Wrapper for list with maximun node caps.
	\note - Implement true lru, currently this is almost like a FIFO, if the caller dont call ReferenceLruNode, then it is true FIFO
*/

#include <ds/lrulist.h>

/*! This is used to initialize a lru list
	\param lru - least recently used list
	\param maximum_count - maximum nodes allowed on this lru
	\param AllocateNode - function pointer to allocate a new node for the lru
	\param FreeNode - function pointer to reuse a node from the lru
	\param FreeNode - function pointer to free a node from the lru
*/
void InitLruList(LRU_LIST_PTR lru, long maximum_count, LIST_PTR (*AllocateNode)(), void (*ReuseNode)(LIST_PTR node),  void (*FreeNode)(LIST_PTR node))
{
	lru->maximum_count = maximum_count;
	
	lru->AllocateNode = AllocateNode;
	lru->ReuseNode = ReuseNode;
	lru->FreeNode = FreeNode;
	
	lru->current_count = 0;
	lru->list_head = NULL;
	
	InitSpinLock( &lru->lock );
}
/*! Returns a node from the lru list
	If current count is less than maximum allowed then really allocate node by calling the given allocator routine.
	Else use the least recently used node(tail)
	\param lru - LRU list
*/
LIST_PTR AllocateLruNode(LRU_LIST_PTR lru)
{
	LIST_PTR node = NULL;
	assert( lru != NULL );
	
	SpinLock( &lru->lock );
	
	if (  lru->current_count <= lru->maximum_count  )
	{
		node = lru->AllocateNode();
		InitList( node );
		if ( node == NULL )
			goto done;
		lru->current_count++;
	}
	else
	{
		node = lru->list_head->prev;
		RemoveFromList( node );
		if ( lru->ReuseNode != NULL )
		{
			lru->ReuseNode( node );
			InitList( node );
		}
	}
	if ( lru->list_head != NULL )
		AddToList( lru->list_head, node );
	else
		lru->list_head = node;

done:
	SpinUnlock( &lru->lock );
	return node;
}

/*! Free a node from the LRU list
	\param lru - LRU list
	\param node - node to free
*/
void FreeLruNode(LRU_LIST_PTR lru, LIST_PTR node)
{
	assert( lru != NULL && node != NULL );
	assert( lru->current_count > 0 );
	
	SpinLock( &lru->lock );
	
	if ( lru->list_head == node )
	{
		if ( IsListEmpty( node ) )
			lru->list_head = NULL;
		else
			lru->list_head = node->next;
	}
		
	RemoveFromList( node );
	lru->current_count--;
	lru->FreeNode( node );
		
	SpinUnlock( &lru->lock );
}
/*! Move the given list node to head of the lru so that it became recently referenced/accessed node
	\param lru - LRU list
	\param node - node in the LRU list to reference
*/
void ReferenceLruNode(LRU_LIST_PTR lru, LIST_PTR node)
{
	assert( lru != NULL && node != NULL );
	
	SpinLock( &lru->lock );
	if ( node != lru->list_head )
	{
		RemoveFromList( node );
		AddToList( lru->list_head, node );
	}
 	SpinUnlock( &lru->lock );
}
