/*!
	\file		src/kernel/mm/vm_unit.c	
	\author		Samuel
	\version 	3.0
	\date	
  			Created: 02-Jun-2008 10:34pm
  			Last modified: 02-Jun-2008 10:34pm
	\brief	vm unit related routines
*/
#include <string.h>
#include <ds/avl_tree.h>
#include <kernel/mm/vm.h>
#include <kernel/mm/kmem.h>

/*! Initializes the given VM unit
*/
void InitVmUnit(VM_UNIT_PTR unit, UINT32 type, UINT32 size)
{
	UINT32 total_vtop = size / PAGE_SIZE;
	assert( unit != NULL );

	InitSpinLock( &unit->lock );
	unit->reference_count = 0;
	
	unit->type = type;
	unit->flag = 0;
	
	unit->size = size;
	
	InitSpinLock(&unit->vtop_lock);
	unit->vtop_array = (VM_VTOP_PTR)kmalloc( sizeof(VM_VTOP) * total_vtop, KMEM_NO_FAIL ) ;
	unit->page_count = 0;
}

VM_UNIT_PTR CreateVmUnit(UINT32 type, UINT32 size)
{
	VM_UNIT_PTR vu = (VM_UNIT_PTR)kmalloc(sizeof(VM_UNIT), KMEM_NO_FAIL);
	InitVmUnit( vu, type, size );
	return vu;
}
