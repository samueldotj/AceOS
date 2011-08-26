/*!
	\file	kernel/iom/iom.c
	\brief	Ace IO Manager.
*/
#include <ace.h>
#include <string.h>
#include <ctype.h>
#include <kernel/debug.h>
#include <kernel/mm/vm.h>
#include <kernel/mm/kmem.h>
#include <kernel/pm/elf.h>
#include <kernel/iom/iom.h>
#include <kernel/vfs/vfs.h>
#include <kernel/iom/devfs.h>

#define DEVICE_OBJECT_CACHE_FREE_SLABS_THRESHOLD	50
#define DEVICE_OBJECT_CACHE_MIN_BUFFERS				50
#define DEVICE_OBJECT_CACHE_MAX_SLABS				1000

#define DRIVER_OBJECT_CACHE_FREE_SLABS_THRESHOLD	10
#define DRIVER_OBJECT_CACHE_MIN_BUFFERS				10
#define DRIVER_OBJECT_CACHE_MAX_SLABS				100

#define IRP_CACHE_FREE_SLABS_THRESHOLD				50
#define IRP_CACHE_MIN_BUFFERS						50
#define IRP_CACHE_MAX_SLABS							1000

/*! List of drivers loaded into the kernel address space */
LIST_PTR driver_list_head = NULL;

/*! cache for driver and device*/
CACHE driver_object_cache;
CACHE device_object_cache;
/*! cache for irps*/
CACHE irp_cache;

extern DRIVER_OBJECT_PTR LoadDriver(char * device_id);
extern ERROR_CODE FindDriverFile(char * device_id, char * buffer, int buf_length);
static ERROR_CODE DummyMajorFunction(DEVICE_OBJECT_PTR pDeviceObject, IRP_PTR pIrp);
int DriverObjectCacheConstructor( void * buffer);
int DriverObjectCacheDestructor( void *buffer);
int DeviceObjectCacheConstructor( void *buffer);
int DeviceObjectCacheDestructor( void *buffer);
int IrpCacheConstructor( void * buffer);
int IrpCacheDestructor( void * buffer);

extern void InitDevFs();


/*! Initialize IO manager and start required boot drivers*/
void InitIoManager()
{
	DRIVER_OBJECT_PTR root_bus;

	/*initialize cache objects for io manager*/
	if( InitCache(&driver_object_cache, sizeof(DRIVER_OBJECT), DRIVER_OBJECT_CACHE_FREE_SLABS_THRESHOLD, DRIVER_OBJECT_CACHE_MIN_BUFFERS, DRIVER_OBJECT_CACHE_MAX_SLABS, DriverObjectCacheConstructor, DriverObjectCacheDestructor) )
		panic("InitIoManager - Driver object cache init failed");
	
	if( InitCache(&device_object_cache, sizeof(DEVICE_OBJECT), DEVICE_OBJECT_CACHE_FREE_SLABS_THRESHOLD, DEVICE_OBJECT_CACHE_MIN_BUFFERS, DEVICE_OBJECT_CACHE_MAX_SLABS, DeviceObjectCacheConstructor, DeviceObjectCacheDestructor) )
		panic("InitIoManager - Device object cache init failed");
	
	if( InitCache(&irp_cache, sizeof(IRP), IRP_CACHE_FREE_SLABS_THRESHOLD, IRP_CACHE_MIN_BUFFERS, IRP_CACHE_MAX_SLABS, IrpCacheConstructor, IrpCacheDestructor) )
		panic("InitIoManager - IRP cache init failed");
		
	/*initialize dev fs*/
	InitDevFs();
	
	/*load root bus driver and call the DriverEntry*/
	root_bus = LoadRootBusDriver() ;

	/*this is the first driver loaded into the kernel and it is never unloaded*/
	driver_list_head = &root_bus->driver_list;
	RootBusDriverEntry( root_bus );

	/*create device object for root bus*/
	CreateDevice(root_bus_driver_object, 0, &root_bus_device_object, NULL, DO_BUFFERED_IO);

	/*force the io manager to enumerate the buses on root bus*/
	InvalidateDeviceRelations(root_bus_device_object, DEVICE_RELATIONS_TYPE_BUS_RELATION);
}

/*! Dummy Major function handler - used to initialize driver object function pointers
	Just returns failure
*/
static ERROR_CODE DummyMajorFunction(DEVICE_OBJECT_PTR pDeviceObject, IRP_PTR pIrp)
{
	assert( pDeviceObject != NULL );
	assert( pIrp != NULL );
	
	pIrp->io_status.status = ERROR_NOT_SUPPORTED;
	
	return ERROR_NOT_SUPPORTED;
}

/*! Creates a device object for use by the driver
	\param driver_object - caller's driver object - this is used by io manager to link the created device with the driver
	\param device_extension_size - size of the device_externsion - io manager allocates this much byte in the device_object
	\param device_object - pointer to device object - io manager updates this pointer with the newly created device object
	\param device_name - optional - device file name - it will be placed under /device/xxxx - applications use this file to communicate with the driver
*/
ERROR_CODE CreateDevice(DRIVER_OBJECT_PTR driver_object, UINT32 device_extension_size, DEVICE_OBJECT_PTR * device_object, char * device_name, UINT32 flag)
{
	DEVICE_OBJECT_PTR dob;
	assert( device_object != NULL );
	
	dob = AllocateBuffer( &device_object_cache, CACHE_ALLOC_SLEEP );
	if ( dob == NULL )
		return ERROR_NOT_ENOUGH_MEMORY;
	dob->driver_object = driver_object;
	if ( device_extension_size )
		dob->device_extension = kmalloc( device_extension_size, KMEM_NO_FAIL );
		
	/*add to the driver's device list*/
	if ( driver_object->device_object_head == NULL )
		driver_object->device_object_head = dob;
	else
		AddToListTail( &driver_object->device_object_head->device_object_list, &dob->device_object_list );
		
	/*create a device file if name is provided*/
	if ( device_name )
	{
		ERROR_CODE ret;
		ret = CreateDeviceNode(device_name, dob);
		/*\todo - do we need to care about CreateDeviceNode return status?*/
	}
	
	/*\todo - validate the flag*/
	dob->flags = flag;
	
	*device_object = dob;
	
	/*force the io manager to enumerate child devices */
	//InvalidateDeviceRelations(dob, DEVICE_RELATIONS_TYPE_BUS_RELATION);
	
	return ERROR_SUCCESS;
}

/*! Links the given source device to target device at higher level, such that any IRP passed to source device can be passed down to target_device
	\param source_device - parent device
	\param target_device - child device
	\return parent device
*/
DEVICE_OBJECT_PTR AttachDeviceToDeviceStack(DEVICE_OBJECT_PTR source_device, DEVICE_OBJECT_PTR target_device)
{
	DEVICE_OBJECT_PTR parent_device = target_device;
	
	assert( source_device->parent_device == NULL );
	
	if( target_device->child_device == NULL )
	{
		/*if this is the first child, set the child_device pointer*/
		target_device->child_device = source_device;
	}
	else
	{
		/*target device already has a one or more child devices, so add this device to the tail*/
		AddToListTail(&target_device->child_device->sibilings_list, &source_device->sibilings_list );
	}
	/*set the source device's parent*/
	source_device->parent_device = parent_device;
	source_device->stack_count = parent_device->stack_count+1;
	return parent_device;
}

/*! Invalidates the device relations, so that IO manager will Query the driver for new relations
	\param device_object - device object on which relations should be invalidates
	\param type - type of the invalidation
*/
void InvalidateDeviceRelations(DEVICE_OBJECT_PTR device_object, DEVICE_RELATION_TYPE type)
{
	DEVICE_RELATIONS_PTR dr;
	IRP_PTR irp;
	int i;

	assert( device_object!= NULL );
	/*bus relations*/
	if( type == DEVICE_RELATIONS_TYPE_BUS_RELATION )
	{
		/*todo - build existing device relations*/

		/*send query relation irp to the driver*/
		irp = AllocateIrp( device_object->stack_count );
		FillIoStack( irp->current_stack_location, IRP_MJ_PNP, IRP_MN_QUERY_DEVICE_RELATIONS, device_object, NULL, NULL);
		irp->current_stack_location->parameters.query_device_relations.type = DEVICE_RELATIONS_TYPE_BUS_RELATION;
		CallDriver(device_object, irp);
		if ( irp->io_status.status != ERROR_SUCCESS )
			goto done;
		dr = (DEVICE_RELATIONS_PTR)irp->io_status.information;
		/*loop through the list of devices call drivers of them to add newly found/created devices*/
		for(i=0; i<dr->count; i++)
		{
			/*\todo - if the device is not new - continue*/
			/*send query id irp for each new device to the driver*/
			ReuseIrp( irp, ERROR_NOT_SUPPORTED );
			FillIoStack( irp->current_stack_location, IRP_MJ_PNP, IRP_MN_QUERY_ID, dr->objects[i], NULL, NULL );
			irp->current_stack_location->parameters.query_id.id_type = BUS_QUERY_DEVICE_ID;
			CallDriver( dr->objects[i] , irp);
			if( irp->io_status.status == ERROR_SUCCESS )
			{
				DRIVER_OBJECT_PTR driver_object;
				driver_object = LoadDriver(irp->io_status.information);
				if ( driver_object != NULL )
				{
					driver_object->fn.AddDevice( driver_object, dr->objects[i] );
				}
				/*free driver id*/
				kfree( irp->io_status.information );
			}
			else
			{
				KTRACE("BUS_QUERY_DEVICE_ID failed\n");
			}
		}
		/*free the device relation struture*/
		kfree( dr );
	}
done:
	FreeIrp(irp);
}

/*! Set a completion routine a lower level driver completes the IRP
 * \param irp
 * \param completion_routine - routine to call
 * \param context - argument to pass to the completion routine
 * \param invoke_on_success - call on success
 * \param invoke_on_error - call on error
 * \param invoke_on_cancel - call on cancel
 * */
ERROR_CODE SetIrpCompletionRoutine(IRP_PTR irp, IO_COMPLETION_ROUTINE completion_routine, void * context, IRP_COMPLETION_INVOKE invoke_on)
{
	assert( irp != NULL );
	assert( irp->current_stack_location != NULL );
	
	irp->current_stack_location->completion_routine = completion_routine;
	irp->current_stack_location->context = context;
	irp->current_stack_location->invoke_on = invoke_on;
	
	return ERROR_SUCCESS;
}

/*! Dispatches a call to driver based on the given Irp
	\param device_object - device object
	\param irp - irp
*/
ERROR_CODE CallDriver(DEVICE_OBJECT_PTR device_object, IRP_PTR irp)
{
	assert( device_object != NULL );
	assert( irp != NULL );
	
	//KTRACE("CallDriver %p %s %s\n", device_object, device_object->driver_object->driver_name, FindKernelSymbolByAddress( device_object->driver_object->fn.MajorFunctions[irp->current_stack_location->major_function], NULL) );
	device_object->driver_object->fn.MajorFunctions[irp->current_stack_location->major_function](device_object, irp);

	return ERROR_SUCCESS;
}


/*! Internal function used to initialize the driver object structure*/
int DriverObjectCacheConstructor( void * buffer)
{
	int i;
	DRIVER_OBJECT_PTR dop = (DRIVER_OBJECT_PTR) buffer;
	
	memset(dop, 0, sizeof(DRIVER_OBJECT) );

	InitList( &dop->driver_list );
	InitSpinLock( &dop->lock );
	for(i=0;i<IRP_MJ_MAXIMUM_FUNCTION;i++)
		dop->fn.MajorFunctions[i] = DummyMajorFunction;
	
	return 0;
}
/*! Internal function used to clear the driver object structure*/
int DriverObjectCacheDestructor( void *buffer)
{
	DriverObjectCacheConstructor( buffer );
	return 0;
}
/*! Internal function used to initialize the device object structure*/
int DeviceObjectCacheConstructor( void *buffer)
{
	DEVICE_OBJECT_PTR dop = (DEVICE_OBJECT_PTR) buffer;
	
	memset(buffer, 0, sizeof(DEVICE_OBJECT) );
	
	InitSpinLock( &dop->lock );
	InitList( &dop->sibilings_list );
	InitList( &dop->device_object_list );
	
	dop->stack_count = 1;
	
	return 0;
}

/*! Internal function used to clear the device object structure*/
int DeviceObjectCacheDestructor( void *buffer)
{
	DeviceObjectCacheConstructor( buffer );
	return 0;
}

/*! Internal function used to initialize the IRP structure*/
int IrpCacheConstructor( void * buffer)
{
	memset(buffer, 0, sizeof(IRP) );

	return 0;
}

/*! Internal function used to initialize the IRP structure*/
int IrpCacheDestructor( void * buffer)
{
	IrpCacheConstructor(buffer);
	
	return 0;
}
