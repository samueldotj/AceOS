/*!
	\file		src/kernel/mm/vm_unit.c	
	\brief	vm unit related routines
*/
#include <string.h>
#include <ds/avl_tree.h>
#include <kernel/mm/vm.h>
#include <kernel/mm/kmem.h>
#include <kernel/mm/pmem.h>
#include <kernel/debug.h>

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

void SetVmUnitPage(VM_UNIT_PTR unit, VIRTUAL_PAGE_PTR vp, UINT32 vtop_index)
{
	assert(unit != NULL);
	assert(vtop_index <= (unit->size / PAGE_SIZE));
	assert(vp != NULL);
	
	unit->page_count++;
	unit->vtop_array[vtop_index].vpage = (VIRTUAL_PAGE_PTR) ( ((VADDR)vp) | 1 );
}

VM_UNIT_PTR CopyVmUnit(VM_UNIT_PTR unit, VADDR start, VADDR end)
{
	VM_UNIT_PTR new_unit;
	UINT32 new_size;
	int i, total_pages, old_start_index;
	
	new_size = end - start;
	
	assert(unit != NULL);
	assert(start < unit->size && IS_PAGE_ALIGNED(start));
	assert(end <= unit->size && IS_PAGE_ALIGNED(end));
	assert(new_size != 0 && new_size <= unit->size);
	
	new_unit = CreateVmUnit(unit->type, 0, new_size);
	
	total_pages = new_size / PAGE_SIZE;
	old_start_index = start/PAGE_SIZE;
	for(i = 0; i < total_pages; i++)
	{
		new_unit->vtop_array[i].vpage = unit->vtop_array[old_start_index+i].vpage;
		if (new_unit->vtop_array[i].in_memory)
		{
			new_unit->page_count++;
			MarkPageForCOW( (VIRTUAL_PAGE_PTR)(((VADDR)new_unit->vtop_array[i].vpage) & ~1));
		}
	}
	if (new_unit->vnode)
	{
		AddVmunitToVnodeList(new_unit->vnode, new_unit, unit->offset + start);
	}
	
	return new_unit;
	
}
