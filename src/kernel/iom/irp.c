/*!
	\file	kernel/iom/irp.c
	\brief	Interrupt request packet.
*/
#include <ace.h>
#include <string.h>
#include <ctype.h>
#include <kernel/debug.h>
#include <kernel/mm/kmem.h>
#include <kernel/pm/elf.h>
#include <kernel/iom/iom.h>
#include <kernel/vfs/vfs.h>

extern CACHE irp_cache;

int IrpCacheConstructor( void * buffer);
int IrpCacheDestructor( void * buffer);

/*! Allocates a Irp for the use of driver
	\param stack_size - number of stacks assoicated with this irp
	\return irp
*/
IRP_PTR AllocateIrp(BYTE stack_size)
{
	IRP_PTR irp;
	
	assert( stack_size>0 );
	/*! allocate irp and io stack*/
	irp = AllocateBuffer( &irp_cache, CACHE_ALLOC_SLEEP );
	if ( irp == NULL )
		return NULL;
	irp->current_stack_location = kmalloc( sizeof(IO_STACK_LOCATION)*stack_size, KMEM_NO_FAIL );
	
	irp->stack_count = stack_size;
	irp->io_status.status = ERROR_NOT_SUPPORTED;
	
	return irp;
}

/*! Frees a irp
	\param irp - pointer to irp to be freed
*/
void FreeIrp(IRP_PTR irp)
{
	assert( irp != NULL );
	
	kfree( irp->current_stack_location );
	FreeBuffer( irp, &irp_cache );
}

/*! Reuses a already allocated Irp
	\param irp - pointer to irp
	\param error_code - Io status will be set with this error code
*/
void ReuseIrp(IRP_PTR irp, ERROR_CODE error_code)
{
	BYTE stack_size;
	IO_STACK_LOCATION_PTR io_stack;
	
	assert( irp != NULL );
	
	stack_size = irp->stack_count;
	io_stack = irp->current_stack_location;

	IrpCacheConstructor( irp );

	irp->current_stack_location	= io_stack;
	irp->stack_count = stack_size;
	irp->io_status.status = error_code;
}

/*! Returns the current IO Stack location associated with the given IRP
	\param irp - interrupt request packet
	\return Current IO stack location on the given irp
*/
IO_STACK_LOCATION_PTR GetCurrentIrpStackLocation(IRP_PTR Irp)
{
	return Irp->current_stack_location;
}

/*! Returns the next lower level IO Stack location associated with the given IRP
	\param irp - interrupt request packet
	\return Next IO stack loation on the given irp.
*/
IO_STACK_LOCATION_PTR GetNextIrpStackLocation(IRP_PTR Irp)
{
	assert( Irp->current_stack >0 && Irp->current_stack < Irp->stack_count );
	return Irp->current_stack_location-1;
}

/*Helper routine to fill a Io Stack with given information*/
inline void FillIoStack(IO_STACK_LOCATION_PTR io_stack, BYTE major_function, BYTE minor_function, DEVICE_OBJECT_PTR device_object, IO_COMPLETION_ROUTINE completion_routine, void * context)
{
	assert( io_stack );
	io_stack->major_function = major_function;
	io_stack->minor_function = minor_function;
	io_stack->device_object = device_object;
	io_stack->completion_routine = completion_routine;
	io_stack->context = context;
}
