/*!
	\file	kernel/mm/virtual_map.c	
	\brief	virtual map related routines
*/
#include <string.h>
#include <ds/avl_tree.h>
#include <kernel/mm/vm.h>
#include <kernel/mm/pmem.h>

VIRTUAL_MAP kernel_map;

CACHE virtual_map_cache;
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

/*! Create a virtual map 
	\param start - minimum virtual address range on this map
	\param end - maximum virutal address range on this map
	\return on success pointer to new virtual map
			on failure null
*/
VIRTUAL_MAP_PTR CreateVirtualMap(VADDR start, VADDR end)
{
	VIRTUAL_MAP_PTR vmap;
	
	vmap = AllocateBuffer(&virtual_map_cache, CACHE_ALLOC_SLEEP);
	if ( vmap == NULL )
		return NULL;
	vmap->start = start;
	vmap->end = end;
	
	vmap->physical_map = CreatePhysicalMap(vmap);
	
	return vmap;
}

/*! Internal function used to initialize the */
int VirtualMapCacheConstructor(void * buffer)
{
	VIRTUAL_MAP_PTR vmap = (VIRTUAL_MAP_PTR)buffer;
	memset(buffer, 0, sizeof(VIRTUAL_MAP) );
	
	InitSpinLock( &vmap->lock );
	
	return 0;
}
/*! Internal function used to clear the virtual map structure*/
int VirtualMapCacheDestructor(void * buffer)
{
	VirtualMapCacheConstructor(buffer);
	return 0;
}
