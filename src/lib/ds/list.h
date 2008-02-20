/*! 
	\file list.h
	\brief Generic doubly linked circular list implementation
	\author Samuel & Dilip
	\date 
		Created: 04-Feb-2008 17:24
		Last modified: 04-Feb-2008 17:24
*/

#ifndef LIST__H
#define LIST__H

#include <stdlib.h>

/*! define LIST_LINK_NULLIFY to nullify the list next/prev pointers after freeing.
	This will be usefull in debugging.
*/
#define LIST_LINK_NULLIFY

/*! List data structure
*/
typedef struct list LIST, * LIST_PTR;
struct list {
	LIST_PTR next;
	LIST_PTR prev;
};

#define INIT_LIST(name) { &name, &name }
#define LIST_NODE(name) LIST name = INIT_LIST(name)


#define STRUCT_FROM_MEMBER(struct_name, field, field_addr )\
	( (struct_name)( (char *)(field_addr) - (char *) &((struct_name)0)->field ) )


/*! To iterate a list use this macro.
*/
#define ListForEach(pos, head) \
	for ((pos) = (head)->next; (pos) != (head) ; (pos) = (pos)->next)

/*! To iterate a list backwards use this macro.
*/
#define ListForEachPrev(pos, head) \
	for ((pos) = (head)->prev; (pos) != (head); (pos) = (pos)->prev)

/*!
 * list_for_each_safe	-	iterate over a list safe against removal of list entry
 * @pos:	the &struct list_head to use as a loop counter.
 * @n:		another &struct list_head to use as temporary storage
 * @head:	the head for your list.
 */
#define ListForEachSafe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; (head) && pos && pos != (head); \
		pos = n, n = pos->next)
		
#ifdef __cplusplus
    extern "C" {
#endif

void InitList(LIST_PTR head);
int IsListEmpty(LIST_PTR node);
void AddToListTail(LIST_PTR head, LIST_PTR new_node);
void AddToList(LIST_PTR head, LIST_PTR new_node);
void RemoveFromList(LIST_PTR node);
void LinkLists(LIST_PTR list1_tail, LIST_PTR list2_head);
	
#ifdef __cplusplus
	}
#endif


#endif
