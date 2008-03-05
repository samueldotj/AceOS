/*!
	\file		list.c
	\author		Samuel & Dilip
	\version 	1.0
	\date	
  			Created: 04-Feb-2008 17:24
  			Last modified: Wed Mar 05, 2008  03:57PM
	\brief	Generic doubly linked circular list implementation
	
	1. There is no null termination because its doubly linked and circular list
	2. Each node should be initialized by calling InitList() or should defined using LIST_NODE macro
	3. To add AddToList and AddToListTail can be used, they add the new node next and previous to the head respectively.
	4. There is no function to add a node as head, if necessary the caller after adding the node should the head pointer.
	5. To remove call RemoveFromList
	6. To iterate through the list you can use ListForEach or ListForEachPrev macro		
*/

#include <list.h>

/*! This is used to initialize a circular doubly linked list
	Use the LIST_NODE macro if possible
*/
void InitList(LIST_PTR head)
{
	head->next = head->prev = head;
}

int IsListEmpty(LIST_PTR node)
{
	return node->next == node;
}
/*! This function will link the given 3 nodes
		ie it will create link like     prev <===> node <====> next
		it does not modify prev->prev and next->next pointers.
*/
static void LinkNodes(LIST_PTR prev, LIST_PTR node, LIST_PTR next)
{
	assert( prev!=NULL );
	assert( node != NULL );
	assert( next!=NULL );

	node->next = next;
	node->prev = prev;
	
	prev->next = node;
	next->prev = node;
}

/*! Insert the new node in the list after the head
*/
void AddToList(LIST_PTR head, LIST_PTR new_node)
{
	assert( head!=NULL );
	assert( new_node != NULL );

	LinkNodes( head, new_node, head->next);
}
/*! Insert the new node in the list before the head
*/
void AddToListTail(LIST_PTR head, LIST_PTR new_node)
{
	assert( head!=NULL );
	assert( new_node != NULL );

	LinkNodes( head->prev, new_node, head);
}

/*!	Delete the node from the list.
 */
void RemoveFromList(LIST_PTR node)
{
	assert( node != NULL );

	node->prev->next = node->next;
	node->next->prev = node->prev;
#ifdef LIST_LINK_NULLIFY
	printf("inside here...........\n");
//		node->next = node->prev = NULL;
#endif
}

void LinkLists(LIST_PTR list1_tail, LIST_PTR list2_head)
{
	LIST_PTR list1_head, list2_tail;
	list1_head = list1_tail->next;
	list2_tail = list2_head->prev;
	
	list1_head->prev = list2_tail;
	list2_tail->next = list1_head;
	
	list1_tail->next = list2_head;
	list2_head->prev = list1_tail;
}


