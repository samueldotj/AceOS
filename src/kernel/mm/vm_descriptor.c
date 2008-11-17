/*!
	\file		src/kernel/mm/vm_descriptor.c	
	\brief	vm descriptor related routines
*/
#include <string.h>
#include <ds/avl_tree.h>
#include <kernel/mm/vm.h>
#include <kernel/mm/pmem.h>
#include <kernel/mm/kmem.h>

static COMPARISION_RESULT compare_vm_descriptor_with_va(struct binary_tree * node1, struct binary_tree * node2);
static COMPARISION_RESULT compare_vm_descriptor(struct binary_tree * node1, struct binary_tree * node2);
static void * FindVaRange(VM_DESCRIPTOR_PTR descriptor_ptr, VADDR start, UINT32 size, int top_down_search, VADDR last_va_end);

/*! Initializes the given VM descriptor structure and adds it to the VM map
*/
void InitVmDescriptor(VM_DESCRIPTOR_PTR descriptor, VIRTUAL_MAP_PTR vmap, VADDR start, VADDR end, VM_UNIT_PTR vm_unit, VM_PROTECTION_PTR protection)
{
	assert( descriptor != NULL );

	InitSpinLock( &descriptor->lock );
	descriptor->reference_count = 0;
	
	descriptor->virtual_map = vmap;
	
	InitAvlTreeNode( &descriptor->tree_node, 0 );
	
	descriptor->start = start;
	descriptor->end = end;
	descriptor->offset_in_unit = 0;
	
	memmove( &descriptor->protection, protection, sizeof(VM_PROTECTION) );
		
	descriptor->unit = vm_unit;
	InsertNodeIntoAvlTree(&vmap->descriptors, &descriptor->tree_node, 0, compare_vm_descriptor);
	vmap->descriptor_count++;
}

VM_DESCRIPTOR_PTR CreateVmDescriptor(VIRTUAL_MAP_PTR vmap, VADDR start, VADDR end, VM_UNIT_PTR vm_unit, VM_PROTECTION_PTR protection)
{
	VM_DESCRIPTOR_PTR vd = (VM_DESCRIPTOR_PTR)kmalloc(sizeof(VM_DESCRIPTOR), KMEM_NO_FAIL);
	InitVmDescriptor( vd, vmap, start, end, vm_unit, protection);
	SpinLock(&vm_unit->lock);
	vm_unit->reference_count++;
	SpinUnlock(&vm_unit->lock);
	vd->reference_count++;
	return vd;
}

/*! Finds a free VM range and returns the starting address.
	\param vmap - virtual map to search for free range
	\param start - optional parameter to start the search (if 0 it is ignored)
	\param size - required size of the free range
	\param option - An option to search VA range 
	\return starting free virtual address on success
		  NULL on failure.
*/
void * FindFreeVmRange(VIRTUAL_MAP_PTR vmap, VADDR start, UINT32 size, UINT32 option)
{
	VADDR end = NULL;
	
 	assert( vmap!=NULL );
	assert( size > 0 );
	/*try to find a hole which has enough space*/
	end = (VADDR)FindVaRange( STRUCT_ADDRESS_FROM_MEMBER( vmap->descriptors, VM_DESCRIPTOR, tree_node), start, size, option & VA_RANGE_SEARCH_FROM_TOP,  vmap->start);
	/*if no hole found try to allocate at the end of virtual address map*/
	if ( end == NULL )
	{
		if (vmap == &kernel_map) 
		{
			if ( (KERNEL_MAP_END_VA - vmap->end) >= size )
			{
				end = vmap->end;
				vmap->end += size;
			}
		}
		else if ( (USER_MAP_END_VA - vmap->end) >= size )
		{
			end = vmap->end;
			vmap->end += size;
		}
	}
	return (void *)end;
}

/*! Returns the vm descriptor associated for the given VA in the given map
	\param vmap - Virtual map which should be searched for the the VA
	\param va - virtual address 
	\return if the va exists on the map the corresponding vm descriptor else NULL
*/
VM_DESCRIPTOR_PTR GetVmDescriptor(VIRTUAL_MAP_PTR vmap, VADDR va)
{
	VM_DESCRIPTOR_PTR vm_descriptor = NULL;
	VM_DESCRIPTOR search_descriptor;
	AVL_TREE_PTR ret;
	
	search_descriptor.start = PAGE_ALIGN(va);
	search_descriptor.end = search_descriptor.start + PAGE_SIZE;
	ret = SearchAvlTree(vmap->descriptors, &(search_descriptor.tree_node), compare_vm_descriptor_with_va);
	if ( ret )
		vm_descriptor = STRUCT_ADDRESS_FROM_MEMBER( ret, VM_DESCRIPTOR, tree_node );
	
	return vm_descriptor;
}

/*! Traverses throught the vm descriptor range to find a hole
	\param descriptor_ptr - descriptor to start with
	\param start - start address
	\param size - size required
	\param top_down_search - 1 if ascending order or descending order search
	\param last_va_end - used for recurssion (pass 0)
*/
static void * FindVaRange(VM_DESCRIPTOR_PTR descriptor_ptr, VADDR start, UINT32 size, int top_down_search, VADDR last_va_end)
{
	/*need implementation with a AVL TREE using size as key*/
	return NULL;
}

/*! Searches the vm descriptor AVL tree for a particular vm descriptor*/
static COMPARISION_RESULT compare_vm_descriptor(struct binary_tree * node1, struct binary_tree * node2)
{	
	VM_DESCRIPTOR_PTR d1, d2;
	assert( node1 != NULL );
	assert( node2 != NULL );
	
	d1 = STRUCT_ADDRESS_FROM_MEMBER(node1, VM_DESCRIPTOR, tree_node.bintree);
	d2 = STRUCT_ADDRESS_FROM_MEMBER(node2, VM_DESCRIPTOR, tree_node.bintree);

	assert( d1->start != d2->start );
	if ( d1->start < d2->start )
		return GREATER_THAN;
	else 
		return LESS_THAN;
}

/*! Searches the vm descriptor AVL tree for a particular VA range*/
static COMPARISION_RESULT compare_vm_descriptor_with_va(struct binary_tree * node1, struct binary_tree * node2)
{
	VM_DESCRIPTOR_PTR d1, d2;
	assert( node1 != NULL );
	assert( node2 != NULL );
	
	d1 = STRUCT_ADDRESS_FROM_MEMBER(node1, VM_DESCRIPTOR, tree_node.bintree);
	d2 = STRUCT_ADDRESS_FROM_MEMBER(node2, VM_DESCRIPTOR, tree_node.bintree);

	if( d1->start <= d2->start && d1->end >= d2->end )
		return EQUAL;

	if ( d1->start < d2->start )
		return GREATER_THAN;
	else 
		return LESS_THAN;
}

