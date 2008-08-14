/*! 
	\file 	ds/list.h
	\brief 	Generic doubly linked circular list implementation
*/

#ifndef LIST__H
#define LIST__H

#include <stdlib.h>
#include <assert.h>

/*! define LIST_LINK_NULLIFY to nullify the list next/prev pointers after freeing.
	This will be usefull in debugging.
*/
//#define LIST_LINK_NULLIFY

/*! List data structure
*/
typedef struct list LIST, * LIST_PTR;
struct list {
	LIST_PTR next;
	LIST_PTR prev;
};

#define INIT_LIST(name) { &name, &name }
#define LIST_NODE(name) LIST name = INIT_LIST(name)


/*! To iterate a list use this macro.
*/
#define LIST_FOR_EACH(pos, head) \
	for ((pos) = (head)->next; (pos) != (head) ; (pos) = (pos)->next)

/*! To iterate a list backwards use this macro.
	\param pos - the &struct list_head to use as a loop counter.
	\param head - the head for your list.
*/
#define LIST_FOR_EACH_PREV(pos, head) \
	for ((pos) = (head)->prev; (pos) != (head); (pos) = (pos)->prev)

/*! iterate over a list safe against removal of list entry
	\param pos - the &struct list_head to use as a loop counter.
	\param n - another &struct list_head to use as temporary storage
	\param head - the head for your list.
*/
#define LIST_FOR_EACH_SAFE(pos, n, head) \
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
