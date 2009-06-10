/*!
	\file		src/kernel/mm/vm_descriptor.c	
	\brief	vm descriptor related routines
*/
#include <string.h>
#include <ds/bits.h>
#include <ds/avl_tree.h>
#include <kernel/debug.h>
#include <kernel/mm/vm.h>
#include <kernel/mm/pmem.h>
#include <kernel/mm/kmem.h>

/*! argument structure for enumerating vm descriptors*/
typedef struct enumerate_descriptor_arg
{
	VADDR	preferred_start;					/*! IN - Preferred start address*/
	VADDR	size;								/*! IN - Size required*/
	
	VADDR	result;								/*! OUT - Start of free virtual address range of required size*/
	
	VADDR	previous_descriptor_va_start;		/*! internal use - last used va range start*/
	VADDR	previous_descriptor_va_end;			/*! internal use - last used va range end*/
}ENUMERATE_DESCRIPTOR_ARG, * ENUMERATE_DESCRIPTOR_ARG_PTR;

static COMPARISION_RESULT compare_vm_descriptor_with_va(struct binary_tree * node1, struct binary_tree * node2);
static COMPARISION_RESULT compare_vm_descriptor(struct binary_tree * node1, struct binary_tree * node2);
static void * FindVaRange(VM_DESCRIPTOR_PTR descriptor_ptr, VADDR start, UINT32 size, int top_down_search, VADDR last_va_end);
static int enumerate_descriptor_callback(AVL_TREE_PTR node, void * arg);

CACHE vm_descriptor_cache;

/*! Initializes the given VM descriptor structure and adds it to the VM map
	\param descriptor - vm descriptor to be initialized
	\param vmap - virtual map of this descriptor
	\param start - starting va address of this descriptor range
	\param end - ending va address of this descriptor range
	\param vm_unit - vm unit which is backing this descriptor
	\param protection - protection for this range
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

/*! Creates and initalizes a vm descriptor
	\param vmap - virtual map of this descriptor
	\param start - starting va address of this descriptor range
	\param end - ending va address of this descriptor range
	\param vm_unit - vm unit which is backing this descriptor
	\param protection - protection for this range
	\return on success pointer to vm descriptor
			on failure null
*/
VM_DESCRIPTOR_PTR CreateVmDescriptor(VIRTUAL_MAP_PTR vmap, VADDR start, VADDR end, VM_UNIT_PTR vm_unit, VM_PROTECTION_PTR protection)
{
	VM_DESCRIPTOR_PTR vd;
	
	SpinLock(&vmap->lock);
	vmap->reference_count++;
	SpinUnlock(&vmap->lock);
	
	vd = (VM_DESCRIPTOR_PTR)kmalloc(sizeof(VM_DESCRIPTOR), KMEM_NO_FAIL);
	//vd = AllocateBuffer( &vm_descriptor_cache, 0 );
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
	VADDR result = NULL;
	
 	assert( vmap!=NULL );
	assert( size > 0 );
	
	/*sanity check*/
	if ( start+size <= start )
		return NULL;

	/*if there is no vm descriptors - we can just use the prefered start*/
	if ( vmap->descriptors == NULL )
	{
		/*return failure if the prefered start is outside boundary*/
		if ( start < USER_MAP_START_VA || start > USER_MAP_END_VA || start+size > USER_MAP_END_VA )
		{
			return NULL;
		}
		result = start;
	}
	else
	{
		/*try to find whether we can use preferred address*/
		if( GetVmDescriptor(vmap, start, size) == 0 )
		{
			result = start;
		}
		else
		{
			/*try to find a hole which has enough space*/
			result = (VADDR)FindVaRange( STRUCT_ADDRESS_FROM_MEMBER( vmap->descriptors, VM_DESCRIPTOR, tree_node), start, size, option & VA_RANGE_SEARCH_FROM_TOP,  vmap->start);
		}
	}
	/*if no hole found try to allocate at the end of virtual address map*/
	if ( result == NULL )
	{
		if ( ( vmap == &kernel_map && (KERNEL_MAP_END_VA - vmap->end) >= size ) ||
			 ( vmap != &kernel_map && (USER_MAP_END_VA   - vmap->end) >= size ) )
		{
				result = vmap->end;
		}
	}
	return (void *)result;
}

/*! Returns the vm descriptor associated for the given VA in the given map
	\param vmap - Virtual map which should be searched for the the VA
	\param va - virtual address 
	\param size - size of the virtual address range
	\return if the va exists on the map the corresponding vm descriptor else NULL
*/
VM_DESCRIPTOR_PTR GetVmDescriptor(VIRTUAL_MAP_PTR vmap, VADDR va, UINT32 size)
{
	VM_DESCRIPTOR_PTR vm_descriptor = NULL;
	VM_DESCRIPTOR search_descriptor;
	AVL_TREE_PTR ret;
	
	assert(size >=1 );
	
	search_descriptor.start = va;
	search_descriptor.end = search_descriptor.start + size-1;
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
	ENUMERATE_DESCRIPTOR_ARG arg;
		
	memset( &arg, 0, sizeof(arg) );
	arg.preferred_start = start;
	arg.size = size;
	
	EnumerateAvlTree( &descriptor_ptr->tree_node, enumerate_descriptor_callback, &arg );
	
	return (void *)arg.result;
}
/*! Enumerator - call back function used by FindVaRange()->EnumerateAvlTree()
	\param node - AVL tree node(vm descriptor)
	\param arg - preferred start and size of the required new region
*/
static int enumerate_descriptor_callback(AVL_TREE_PTR node, void * arg)
{
	ENUMERATE_DESCRIPTOR_ARG_PTR a= (ENUMERATE_DESCRIPTOR_ARG_PTR) arg;
	VM_DESCRIPTOR_PTR descriptor = STRUCT_ADDRESS_FROM_MEMBER(node, VM_DESCRIPTOR, tree_node);
	VADDR va_start, va_end, size;
	
	va_start = PAGE_ALIGN(a->preferred_start);
	va_end = PAGE_ALIGN_UP(a->preferred_start+a->size);
	size = PAGE_ALIGN_UP(a->size)-1;
	a->previous_descriptor_va_end = PAGE_ALIGN_UP(a->previous_descriptor_va_end);
	/*check whether the hole has "preferred" start and required size*/
	if ( RANGE_WITH_IN_RANGE( a->previous_descriptor_va_end, descriptor->start, va_start, va_end ) )
	{
		/*update the result with correct address*/
		a->result = va_start;
		/*terminate enumeration*/
		return 1;
	}
	/*atleast the hole has required size?*/
	else if ( (descriptor->start - a->previous_descriptor_va_end) > size )
	{
		a->result = a->previous_descriptor_va_end;
		/*break the enumeration if we passed preferred va range*/
		if ( descriptor->end > a->preferred_start )
			return 1;
	}
	
	a->previous_descriptor_va_start = descriptor->start;
	a->previous_descriptor_va_end = descriptor->end;
	/*continue enumeration*/
	return 0;
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
	if ( d1->start > d2->start )
		return LESS_THAN;		
	else 
		return GREATER_THAN;
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

	if ( d1->start > d2->start )
		return LESS_THAN;
	else
		return GREATER_THAN;
		
}
/*! Internal function used to initialize the vm descriptor structure*/
int VmDescriptorCacheConstructor(void * buffer)
{
	VM_DESCRIPTOR_PTR vd = (VM_DESCRIPTOR_PTR)buffer;
	memset(buffer, 0, sizeof(VM_DESCRIPTOR) );
	
	InitSpinLock( &vd->lock );
	
	return 0;
}
/*! Internal function used to clear the virtual map structure*/
int VmDescriptorCacheDestructor(void * buffer)
{
	VmDescriptorCacheConstructor(buffer);
	return 0;
}

