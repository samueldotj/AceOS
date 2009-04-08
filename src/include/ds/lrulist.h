/*! 
	\file 	ds/lrulist.h
	\brief 	Least recently used list
*/

#ifndef LRU_LIST__H
#define LRU_LIST__H

#include <stdlib.h>
#include <assert.h>
#include <ds/list.h>
#include <sync/spinlock.h>

/*! least recently used list*/
typedef struct lru_list
{
	SPIN_LOCK 	lock;						/*! lock for current_count and list_head*/
	
	long 		maximum_count;				/*! maximum nodes allowed*/
	long 		current_count;				/*! current number of nodes in the list*/
	
	LIST_PTR	(*AllocateNode)();			/*! function pointer to allocate a new node*/
	void		(*ReuseNode)(LIST_PTR node);/*! function pointer to reuse a node*/
	void		(*FreeNode)(LIST_PTR node);	/*! function pointer to free a node*/
	
	LIST_PTR	list_head;					/*! Pointer to starting of the list(most recent)*/
}LRU_LIST, * LRU_LIST_PTR;

#ifdef __cplusplus
    extern "C" {
#endif

void InitLruList(LRU_LIST_PTR lru, long maximum_count, LIST_PTR (*AllocateNode)(), void (*ReuseNode)(LIST_PTR node),  void (*FreeNode)(LIST_PTR node));
LIST_PTR AllocateLruNode(LRU_LIST_PTR lru);
void FreeLruNode(LRU_LIST_PTR lru, LIST_PTR node);
void ReferenceLruNode(LRU_LIST_PTR lru, LIST_PTR node);

#ifdef __cplusplus
	}
#endif

#endif
/*! 
	\file 	ds/lrulist.h
	\brief 	Least recently used list
*/

#ifndef LRU_LIST__H
#define LRU_LIST__H

#include <stdlib.h>
#include <assert.h>
#include <ds/list.h>
#include <sync/spinlock.h>

/*! least recently used list*/
typedef struct lru_list
{
	SPIN_LOCK 	lock;						/*! lock for current_count and list_head*/
	
	long 		maximum_count;				/*! maximum nodes allowed*/
	long 		current_count;				/*! current number of nodes in the list*/
	
	LIST_PTR	(*AllocateNode)();			/*! function pointer to allocate a new node*/
	void		(*ReuseNode)(LIST_PTR node);/*! function pointer to reuse a node*/
	void		(*FreeNode)(LIST_PTR node);	/*! function pointer to free a node*/
	
	LIST_PTR	list_head;					/*! Pointer to starting of the list(most recent)*/
}LRU_LIST, * LRU_LIST_PTR;

#ifdef __cplusplus
    extern "C" {
#endif

void InitLruList(LRU_LIST_PTR lru, long maximum_count, LIST_PTR (*AllocateNode)(), void (*ReuseNode)(LIST_PTR node),  void (*FreeNode)(LIST_PTR node));
LIST_PTR AllocateLruNode(LRU_LIST_PTR lru);
void FreeLruNode(LRU_LIST_PTR lru, LIST_PTR node);
void ReferenceLruNode(LRU_LIST_PTR lru, LIST_PTR node);

#ifdef __cplusplus
	}
#endif

#endif
/*! 
	\file 	ds/lrulist.h
	\brief 	Least recently used list
*/

#ifndef LRU_LIST__H
#define LRU_LIST__H

#include <stdlib.h>
#include <assert.h>
#include <ds/list.h>
#include <sync/spinlock.h>

/*! least recently used list*/
typedef struct lru_list
{
	SPIN_LOCK 	lock;						/*! lock for current_count and list_head*/
	
	long 		maximum_count;				/*! maximum nodes allowed*/
	long 		current_count;				/*! current number of nodes in the list*/
	
	LIST_PTR	(*AllocateNode)();			/*! function pointer to allocate a new node*/
	void		(*ReuseNode)(LIST_PTR node);/*! function pointer to reuse a node*/
	void		(*FreeNode)(LIST_PTR node);	/*! function pointer to free a node*/
	
	LIST_PTR	list_head;					/*! Pointer to starting of the list(most recent)*/
}LRU_LIST, * LRU_LIST_PTR;

#ifdef __cplusplus
    extern "C" {
#endif

void InitLruList(LRU_LIST_PTR lru, long maximum_count, LIST_PTR (*AllocateNode)(), void (*ReuseNode)(LIST_PTR node),  void (*FreeNode)(LIST_PTR node));
LIST_PTR AllocateLruNode(LRU_LIST_PTR lru);
void FreeLruNode(LRU_LIST_PTR lru, LIST_PTR node);
void ReferenceLruNode(LRU_LIST_PTR lru, LIST_PTR node);

#ifdef __cplusplus
	}
#endif

#endif
