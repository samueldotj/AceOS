/*!
	\file		src/kernel/mm/virtual_map.c	
	\author		Samuel
	\version 	3.0
	\date	
  			Created: 02-Jun-2008 10:34pm
  			Last modified: 02-Jun-2008 10:34pm
	\brief	virtual map related routines
*/
#include <string.h>
#include <ds/avl_tree.h>
#include <kernel/mm/vm.h>

VIRTUAL_MAP kernel_map;

/*! Initializes the virtual map data structure.
*/
void InitVirtualMap(VIRTUAL_MAP_PTR vmap, PHYSICAL_MAP_PTR pmap)
{
	assert( vmap != NULL );
	
	InitSpinLock( &vmap->lock );
	vmap->reference_count = 0;
	
	vmap->start = 0;
	vmap->end = 0;
	
	vmap->descriptors = NULL;
	vmap->descriptor_count = 0;
	
	vmap->physical_map = pmap;
}
