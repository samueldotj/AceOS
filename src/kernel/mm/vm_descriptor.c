/*!
	\file		src/kernel/mm/vm_descriptor.c	
	\author		Samuel
	\version 	3.0
	\date	
  			Created: 07-Jul-08 9:42pm
  			Last modified: 07-Jul-08 9:42pm
	\brief	vm descriptor related routines
*/
#include <string.h>
#include <ds/avl_tree.h>
#include <kernel/mm/vm.h>
#include <kernel/mm/pmem.h>
#include <kernel/mm/kmem.h>

static COMPARISION_RESULT compare_vm_descriptor(struct binary_tree * node1, struct binary_tree * node2);
static void * FindVaRange(VM_DESCRIPTOR_PTR descriptor_ptr, VADDR start, UINT32 size, int top_down_search, VADDR last_va_end);
void InitVmDescriptor(VM_DESCRIPTOR_PTR descriptor, VIRTUAL_MAP_PTR vmap, UINT32 start, UINT32 end, VM_UNIT_PTR vm_unit, VM_PROTECTION_PTR protection);

/*! Initializes the given VM descriptor structure and adds it to the VM map
*/
void InitVmDescriptor(VM_DESCRIPTOR_PTR descriptor, VIRTUAL_MAP_PTR vmap, UINT32 start, UINT32 end, VM_UNIT_PTR vm_unit, VM_PROTECTION_PTR protection)
{
	assert( descriptor != NULL );

	InitSpinLock( &descriptor->lock );
	descriptor->reference_count = 0;
	
	descriptor->virtual_map = vmap;
	
	InitAvlTreeNode( &descriptor->tree_node, 0 );
	
	descriptor->start = start;
	descriptor->end = end;
	
	memmove( &descriptor->protection, protection, sizeof(VM_PROTECTION) );
		
	descriptor->unit = vm_unit;
	
	InsertNodeIntoAvlTree(&vmap->descriptors, &descriptor->tree_node, 0, compare_vm_descriptor);
}

VM_DESCRIPTOR_PTR CreateVmDescriptor(VIRTUAL_MAP_PTR vmap, UINT32 start, UINT32 end, VM_UNIT_PTR vm_unit, VM_PROTECTION_PTR protection)
{
	VM_DESCRIPTOR_PTR vd = (VM_DESCRIPTOR_PTR)kmalloc(sizeof(VM_DESCRIPTOR), KMEM_NO_FAIL);
	InitVmDescriptor( vd, vmap, start, end, vm_unit, protection);
	return vd;
}

/*! Finds a free VM range and returns the starting address.
	\param vmap - virtual map to search for free range
	\param start - optional parameter to start the search (if 0 it is ignored)
	\param size - required size of the free range
	\return starting free virtual address on success
		  NULL on failure.
*/
void * FindFreeVmRange(VIRTUAL_MAP_PTR vmap, VADDR start, UINT32 size, UINT32 option)
{
	VADDR end = NULL;

 	assert( vmap!=NULL );
	assert( size > 0 );
	/*try to find a hole which has enough space*/
	end = (VADDR)FindVaRange( STRUCT_ADDRESS_FROM_MEMBER( vmap->descriptors, VM_DESCRIPTOR, tree_node), start, size, option & VA_RANGE_SEARCH_FROM_TOP,  NULL);
	/*if no hole found try to allocate at the end of virtual address map*/
	if ( end == NULL )
	{
		if ( (vmap == &kernel_map) && (KERNEL_MAP_END_VA - vmap->end) >= size )
			end = vmap->end;
		else if ( (USER_MAP_END_VA - vmap->end) >= size )
			end = vmap->end;
	}
	return (void *)end;
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
	void * end=NULL;
	AVL_TREE_PTR next_node;
	
	next_node = NULL;
	if (top_down_search)
	{
		if (! IS_AVL_TREE_LEFT_LIST_END( &descriptor_ptr->tree_node ) )
			next_node = AVL_TREE_LEFT_NODE(&descriptor_ptr->tree_node);
	}
	else
	{
		if (! IS_AVL_TREE_RIGHT_LIST_END( &descriptor_ptr->tree_node ) )
			next_node = AVL_TREE_RIGHT_NODE(&descriptor_ptr->tree_node);
	}
	if ( next_node != NULL )
	{
		end = FindVaRange( STRUCT_ADDRESS_FROM_MEMBER( next_node, VM_DESCRIPTOR, tree_node), start, size, descriptor_ptr->end, top_down_search );
		if ( end )
			return end;
	}
	if ( descriptor_ptr->start > start && size >= (descriptor_ptr->start-last_va_end) )
	{
		return (void *)last_va_end;
	}
	next_node = NULL;
	if (!top_down_search)
	{
		if (! IS_AVL_TREE_LEFT_LIST_END( &descriptor_ptr->tree_node ) )
			next_node = AVL_TREE_LEFT_NODE(&descriptor_ptr->tree_node);
	}
	else
	{
		if (! IS_AVL_TREE_RIGHT_LIST_END( &descriptor_ptr->tree_node ) )
			next_node = AVL_TREE_RIGHT_NODE(&descriptor_ptr->tree_node);
	}
	if ( next_node != NULL )
	{
		end = FindVaRange( STRUCT_ADDRESS_FROM_MEMBER( next_node, VM_DESCRIPTOR, tree_node), start, size, descriptor_ptr->end, top_down_search );
		if ( end )
			return end;
	}
	return NULL;
}
static COMPARISION_RESULT compare_vm_descriptor(struct binary_tree * node1, struct binary_tree * node2)
{	
	VM_DESCRIPTOR_PTR d1, d2;
	assert( node1 != NULL );
	assert( node2 != NULL );
	
	d1 = STRUCT_ADDRESS_FROM_MEMBER(node1, VM_DESCRIPTOR, tree_node);
	d2 = STRUCT_ADDRESS_FROM_MEMBER(node2, VM_DESCRIPTOR, tree_node);

	assert( d1->start != d2->start );
	if ( d1->start < d2->start )
		return GREATER_THAN;
	else 
		return LESS_THAN;
}

