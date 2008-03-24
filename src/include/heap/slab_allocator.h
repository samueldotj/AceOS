/*!
  \file		slab_allocator.h
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created:	Fri Mar 21, 2008  09:04PM
  			Last modified: Mon Mar 24, 2008  12:37PM
  \brief	This file contains structures to maintain slab_allocator
*/


#ifndef _SLAB_ALLOCATOR_H_
#define _SLAB_ALLOCATOR_H_

#include <ace.h>
#include <ds/avl_tree.h>
#include <ds/list.h>
#include <kernel/spinlock.h>

/* Extern declarations go here */

/* Forward declarations go here */

struct allocated_slab_tree;
struct slab;
struct free_buffer_list;
struct cache;

/* Macros go here */

/* This keeps track of size of meta data in a slab */
#define SIZE_SLAB_META_DATA 32

/* temporary definition here.... later will get this from vm */
#define VIRTUAL_PAGE_SIZE 4096

#define GET_META_DATA(slab_addr) (slab_addr) + \
		(((slab_addr)->count_virtual_pages)*VIRTUAL_PAGE_SIZE) - \
		SIZE_SLAB_META_DATA

/* Structure definitions go here */

typedef struct free_buffer_list {
	LIST list;
	struct allocated_slab_tree *slab_node;
} FREE_BUFFER_LIST, *FREE_BUFFER_LIST_PTR;

/* This is the completely free slab */
typedef struct slab {
	LIST list;
	FREE_BUFFER_LIST_PTR free_buffer_list;
	UINT32 count_virtual_pages; /* count of virtual pages */
} SLAB, *SLAB_PTR;

typedef struct allocated_slab_tree {
	AVL_TREE tree;
	SLAB_PTR slab; /* slab this node is representing */
	UINT32 reference_count; /* count of buffers allocated, to free the page */
	UINT32 count_virtual_pages; /* count of virtual_pages, to find meta data */
} ALLOCATED_SLAB_TREE, *ALLOCATED_SLAB_TREE_PTR;


typedef struct cache {
	SPIN_LOCK slock;
	int size; /* Size of buffers available from this cache */

	int (*constructor)(struct cache*, struct slab*); 
	/* points to a function which initializes a newly allocated slab */
	int (*destructor)(struct cache*, struct slab*);
	/* points to a function which makes the slab reusable */

	/* A list of free slabs which can be browsed to get a free buffer for malloc*/
	SLAB_PTR free_slab_list;
	int min_slabs; /* Minimum no of slabs to be present always */
	int max_slabs; /* Maximum no of slabs allowed */
	int free_slabs_count; /* count of free slabs in the free slab list */
	int free_slabs_threshold; /* Threshold to start VM operation */

	/* A tree to store allocated slabs which is used to get the meta data while freeing */
	ALLOCATED_SLAB_TREE_PTR allocated_slab_tree;

	/* A list of free virtual pages which can be used internally or available for VM */
	FREE_BUFFER_LIST_PTR free_buffer_list;
} CACHE, *CACHE_PTR;


/* Function declarations go here */

int CacheCreate (CACHE_PTR new_cache, UINT32 size,
        int free_slabs_threshold, int min_slabs, int max_slabs,
        int (*constructor)(CACHE_PTR, SLAB_PTR),
        int (*destructor)(CACHE_PTR, SLAB_PTR));

void* malloc (UINT32 size, CACHE_PTR *cache_entry);

#endif
