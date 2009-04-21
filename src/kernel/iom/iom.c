/*!
	\file	kernel/iom/iom.c
	\brief	Ace IO Manager.
*/
#include <ace.h>
#include <string.h>
#include <ctype.h>
#include <kernel/debug.h>
#include <kernel/mm/kmem.h>
#include <kernel/pm/elf.h>
#include <kernel/iom/iom.h>
#include <kernel/vfs/vfs.h>

#define DEVICE_OBJECT_CACHE_FREE_SLABS_THRESHOLD	10
#define DEVICE_OBJECT_CACHE_MIN_BUFFERS				10
#define DEVICE_OBJECT_CACHE_MAX_SLABS				10

#define DRIVER_OBJECT_CACHE_FREE_SLABS_THRESHOLD	5
#define DRIVER_OBJECT_CACHE_MIN_BUFFERS				5
#define DRIVER_OBJECT_CACHE_MAX_SLABS				5

#define IRP_CACHE_FREE_SLABS_THRESHOLD				50
#define IRP_CACHE_MIN_BUFFERS						50
#define IRP_CACHE_MAX_SLABS							50

static ERROR_CODE DummyMajorFunction(DEVICE_OBJECT_PTR pDeviceObject, IRP_PTR pIrp);

/*! List of drivers loaded into the kernel address space */
LIST_PTR driver_list_head = NULL;

/*! cache for driver and device*/
CACHE driver_object_cache;
CACHE device_object_cache;
/*! cache for irps*/
CACHE irp_cache;

static DRIVER_OBJECT_PTR LoadDriver(char * device_id);
static ERROR_CODE FindDriverFile(char * device_id, char * buffer, int buf_length);

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

/*! Initialize IO manager and start required boot drivers*/
void InitIoManager()
{
	DRIVER_OBJECT_PTR root_bus;

	if( InitCache(&driver_object_cache, sizeof(DRIVER_OBJECT), DRIVER_OBJECT_CACHE_FREE_SLABS_THRESHOLD, DRIVER_OBJECT_CACHE_MIN_BUFFERS, DRIVER_OBJECT_CACHE_MAX_SLABS, DriverObjectCacheConstructor, DriverObjectCacheDestructor) )
		panic("InitIoManager - Driver object cache init failed");
	
	if( InitCache(&device_object_cache, sizeof(DEVICE_OBJECT), DEVICE_OBJECT_CACHE_FREE_SLABS_THRESHOLD, DEVICE_OBJECT_CACHE_MIN_BUFFERS, DEVICE_OBJECT_CACHE_MAX_SLABS, DeviceObjectCacheConstructor, DeviceObjectCacheDestructor) )
		panic("InitIoManager - Device object cache init failed");
	
	if( InitCache(&irp_cache, sizeof(IRP), IRP_CACHE_FREE_SLABS_THRESHOLD, IRP_CACHE_MIN_BUFFERS, IRP_CACHE_MAX_SLABS, IrpCacheConstructor, IrpCacheDestructor) )
		panic("InitIoManager - IRP cache init failed");
	
	/*load root bus driver and call the DriverEntry*/
	root_bus = LoadRootBusDriver() ;
	
	/*this is the first driver loaded into the kernel and it is never unloaded*/
	driver_list_head = &root_bus->driver_list;

	RootBusDriverEntry( root_bus );

	/*create device object for root bus*/
	CreateDevice(root_bus_driver_object, 0, &root_bus_device_object);
	
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
/*! Creates a device object for use by the driver
	\param driver_object - caller's driver object - this is used by io manager to link the created device with the driver
	\param device_extension_size - size of the device_externsion - io manager allocates this much byte in the device_object
	\param device_object - pointer to device object - io manager updates this pointer with the newly created device object
*/
ERROR_CODE CreateDevice(DRIVER_OBJECT_PTR driver_object, UINT32 device_extension_size, DEVICE_OBJECT_PTR * device_object)
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
	
	*device_object = dob;
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
		/**/
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
				DRIVER_OBJECT_PTR driver_object = LoadDriver(irp->io_status.information);
				if ( driver_object != NULL )
				{
					driver_object->fn.AddDevice( driver_object, dr->objects[i] );
				}
				/*free driver id*/
				kfree( irp->io_status.information );
			}
		}
		/*free the device relation struture*/
		kfree( dr );
	}
done:
	FreeIrp(irp);
}

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

/*! Dispatches a call to driver based on the given Irp
	\param device_object - device object
	\param irp - irp
*/
ERROR_CODE CallDriver(DEVICE_OBJECT_PTR device_object, IRP_PTR irp)
{
	assert( device_object != NULL );
	assert( irp != NULL );
	
	device_object->driver_object->fn.MajorFunctions[irp->current_stack_location->major_function](device_object, irp);
	return ERROR_SUCCESS;
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

/*! Loads a driver*/
static DRIVER_OBJECT_PTR LoadDriver(char * device_id)
{
	char driver_file_name[MAX_FILE_NAME], driver_file_path[MAX_FILE_PATH]="/boot/drivers/";
	VADDR driver_start_address;
	int file_id;
	long file_size;
	DRIVER_OBJECT_PTR driver_object;
	ERROR_CODE (*DriverEntry)(DRIVER_OBJECT_PTR pDriverObject);
	ERROR_CODE err;
	LIST_PTR node;
	
	err = FindDriverFile(device_id, driver_file_name, sizeof(driver_file_name));
	ktrace("Driver for id %s : ", device_id);
	if ( err == ERROR_SUCCESS )
		ktrace("%s\n", driver_file_name);
	else
	{
		ktrace("%s\n", ERROR_CODE_AS_STRING(err) );
		return NULL;
	}		

	/*check whether the driver is already loaded*/
	LIST_FOR_EACH(node, driver_list_head)
	{
		driver_object = STRUCT_ADDRESS_FROM_MEMBER( node, DRIVER_OBJECT, driver_list );
		if ( strcmp( driver_file_name, driver_object->driver_file_name )==0 )
		{
			ktrace("Driver already loaded %s\n", driver_object->driver_file_name);
			return driver_object;
		}
			
	}
	strcat( driver_file_path, driver_file_name );
	kprintf("Loading %s: ", driver_file_path);
	
	err = OpenFile(driver_file_path, VFS_ACCESS_TYPE_READ, OPEN_EXISTING, &file_id);
	if ( err != ERROR_SUCCESS )
		goto error;
	err = GetFileSize(file_id, &file_size);
	if ( err != ERROR_SUCCESS )
		goto error;
	file_size = PAGE_ALIGN_UP(file_size);
	err = MapViewOfFile(file_id, &driver_start_address, PROT_READ, 0, file_size, 0, 0);
	if ( err != ERROR_SUCCESS )
		goto error;
	err = LoadElfImage( (void *) driver_start_address, &kernel_map, "DriverEntry", (void *)&DriverEntry );
	if ( err != ERROR_SUCCESS || DriverEntry == NULL )
		goto error;
	driver_object = AllocateBuffer( &driver_object_cache, CACHE_ALLOC_SLEEP );
	if ( driver_object == NULL )
	{
		err = ERROR_NOT_ENOUGH_MEMORY;
		goto error;
	}
	
	strcpy( driver_object->driver_file_name, driver_file_name);
	err = DriverEntry( driver_object );
	if ( err != ERROR_SUCCESS )
		goto error;

	/*add the driver to the driver list*/
	AddToList( driver_list_head, &driver_object->driver_list );
	kprintf("success\n");
	return driver_object;

error:
	kprintf("%s\n", ERROR_CODE_AS_STRING(err) );
	return NULL;

}

#define SKIP_WHITE_SPACES	while( i<file_size && isspace(va[i]) ) i++;

/*! Finds suitable driver for the given id and returns its full path in the given buffer
	\param device_id - device identification string
	\param buffer - buffer to place the driver path
	\param buf_length - buffer size
*/
static ERROR_CODE FindDriverFile(char * device_id, char * buffer, int buf_length)
{
	ERROR_CODE err;
	int file_id,i=0;
	long file_size;
	char driver_id_database[] = "/boot/driver_id.txt";
	char * va;
	
	assert(device_id != NULL );
	assert(buffer != NULL );
	assert(buf_length > 0);
	
	buffer[0]=0;
	
	err = OpenFile(driver_id_database, VFS_ACCESS_TYPE_READ, OPEN_EXISTING, &file_id);
	if ( err != ERROR_SUCCESS )
		goto done;
		
	err = GetFileSize(file_id, &file_size);
	if ( err != ERROR_SUCCESS )
		goto done;

	err = MapViewOfFile(file_id, (VADDR *) &va, PROT_READ, 0, file_size, 0, 0);
	if ( err != ERROR_SUCCESS )
		goto done;
	
	err=ERROR_NOT_FOUND;
	/*read the file and try to find the driver file name*/
	while(i<file_size)
	{
		char driver_device_id[100];
		SKIP_WHITE_SPACES;
		
		/*skip comments*/
		if( va[i]!='#' )
		{
			int j=0;
			
			/*copy the driver id*/
			while( i<file_size && j<sizeof(driver_device_id)-1 && !isspace(va[i]) )
				driver_device_id[j++] = va[i++];
			driver_device_id[j]=0;
			
			/*if user didnt provide driver file name break*/
			if ( va[i] != ' ' && va[i] != '\t' )
				break;
				
			SKIP_WHITE_SPACES;
			
			if( strcmp(driver_device_id, device_id) == 0 )
			{
				/*copy driver file name*/
				j=0;
				while( i<file_size && j<buf_length-1 && !isspace(va[i]) )
					buffer[j++] = va[i++];
					
				buffer[j]=0;
				err = ERROR_SUCCESS;
				break;
			}
		}
		/*skip till end of line*/
		while( i<file_size && va[i]!='\n') i++;
	}
	
done:
	/* \todo - release the mapping */
	
	CloseFile(file_id);
	return err;
}
