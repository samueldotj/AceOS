/*!
	\file		src/kernel/mm/vm_unit.c	
	\brief	vm unit related routines
*/
#include <string.h>
#include <ds/avl_tree.h>
#include <kernel/mm/vm.h>
#include <kernel/mm/kmem.h>

/*! Initializes the given VM unit
	\param unit - pointer to vm unit
	\param type - vm unit type(anonymous, file mapped etc)
	\param flag - vm unit flag(shared, private)
	\param size - size of the unit
*/
void InitVmUnit(VM_UNIT_PTR unit, VM_UNIT_TYPE type, VM_UNIT_FLAG flag, UINT32 size)
{
	UINT32 total_vtop;
	assert( unit != NULL );

	/*total virtual pages mapped by this unit*/
	total_vtop = (size / PAGE_SIZE) + (size%PAGE_SIZE?1:0) ;
	
	InitSpinLock( &unit->lock );
	unit->reference_count = 0;
	
	unit->type = type;
	unit->flag = flag;
	
	unit->size = size;
	
	InitSpinLock(&unit->vtop_lock);
	unit->vtop_array = (VM_VTOP_PTR)kmalloc( sizeof(VM_VTOP) * total_vtop, KMEM_NO_FAIL ) ;
	
	/*\todo we should provide a way in kmalloc to zero fill returned memory*/
	memset(unit->vtop_array, 0, sizeof(VM_VTOP) * total_vtop);
	unit->page_count = 0;
	
	InitList(&unit->units_in_vnode_list);
	unit->vnode = NULL;
	unit->offset = 0;
}

/*! Create vm unit of given size
	\param unit - pointer to vm unit
	\param type - vm unit type(anonymous, file mapped etc)
	\param flag - vm unit flag(shared, private)
	\param size - size of the unit
*/
VM_UNIT_PTR CreateVmUnit(VM_UNIT_TYPE type, VM_UNIT_FLAG flag, UINT32 size)
{
	VM_UNIT_PTR vu = (VM_UNIT_PTR)kmalloc(sizeof(VM_UNIT), KMEM_NO_FAIL);
	InitVmUnit( vu, type, flag, size );
	return vu;
}

/*! Frees a given vm unit
	\param unit - vm unit to free
*/
void FreeVmUnit(VM_UNIT_PTR unit)
{
	assert( unit != NULL );
	unit->reference_count--;
	if( unit->reference_count > 0 )
		return;
	if( unit->type == VM_UNIT_TYPE_FILE_MAPPED )
	{
		/*\todo remove the unit from vnode and relese the vnode*/
		return;
	}
	if ( unit->page_count )
	{
		/*\todo remove the pages here*/
	}
}
