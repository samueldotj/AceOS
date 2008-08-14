/*!
	\file	kernel/mm/virtual_map.c	
	\brief	virtual map related routines
*/
#include <string.h>
#include <ds/avl_tree.h>
#include <kernel/mm/vm.h>

VIRTUAL_MAP kernel_map;

/*!
 * \brief Initializes the virtual map
 * \param vmap - virtual map to initialize
 * \param pmap - physical map to attach to this virtual map
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
