/*!
	\file	kernel/iom/rootbus.c
	\brief	Imaginary root bus - It has only one device - ACPI bus
*/
#include <ace.h>
#include <stdlib.h>
#include <string.h>
#include <kernel/debug.h>
#include <kernel/iom/iom.h>

#define ACPI_BUS_NAME	"acpi"

typedef struct rootbus_device_extension
{
	char name[DRIVER_NAME_MAX];
}ROOTBUS_DEVICE_EXTENSION, *ROOTBUS_DEVICE_EXTENSION_PTR;

static ERROR_CODE MajorFunctionPnp(DEVICE_OBJECT_PTR pDeviceObject, IRP_PTR pIrp);
/*! Device object for the rootbus
	This can be imagined as device tree which represents all the buses/devices present in the system in the same hierarchy as they present electronically 
*/
DEVICE_OBJECT_PTR root_bus_device_object;

DRIVER_OBJECT_PTR root_bus_driver_object;
/*! Initializes and returns root bus driver*/
DRIVER_OBJECT_PTR LoadRootBusDriver()
{
	root_bus_driver_object = AllocateBuffer( &driver_object_cache, CACHE_ALLOC_SLEEP );
	root_bus_driver_object->driver_file_name[0] = 0;
	return root_bus_driver_object;
}

ERROR_CODE RootBusDriverEntry(DRIVER_OBJECT_PTR pDriverObject)
{
	strcpy( pDriverObject->driver_name, "Root" );
	pDriverObject->driver_extension = NULL;
	pDriverObject->fn.MajorFunctions[IRP_MJ_PNP] = MajorFunctionPnp;

	return ERROR_SUCCESS;
}
static DEVICE_RELATIONS_PTR CreateRootBusDevices(DEVICE_OBJECT_PTR pDeviceObject)
{
	DEVICE_RELATIONS_PTR dr;
	DEVICE_OBJECT_PTR acpi_device_object, console_device_object;
	ROOTBUS_DEVICE_EXTENSION_PTR ext;
	ERROR_CODE err;
	
	/*allocate memory for device relation struction*/
	dr = kmalloc( SIZEOF_DEVICE_RELATIONS(2), 0 );
	if ( dr == NULL )
	{
		panic("Unable to create Root bus devices");
	}
	
	/*create device object for acpi bus*/
	err = CreateDevice( pDeviceObject->driver_object, sizeof(ROOTBUS_DEVICE_EXTENSION), &acpi_device_object, ACPI_BUS_NAME, DO_BUFFERED_IO );
	if ( err != ERROR_SUCCESS )
	{
		panic("Unable to create ACPI bus device");
	}
	
	/*put rootbus specific info to device extension structure*/
	ext = (ROOTBUS_DEVICE_EXTENSION_PTR)acpi_device_object->device_extension;
	strcpy( ext->name, ACPI_BUS_NAME );
	/*attach the acpi device to root bus device io stack*/
	AttachDeviceToDeviceStack(acpi_device_object, pDeviceObject);
	
	/*create device object for acpi bus*/
	err = CreateDevice( pDeviceObject->driver_object, sizeof(ROOTBUS_DEVICE_EXTENSION), &console_device_object, "console_root", DO_BUFFERED_IO );
	if ( err != ERROR_SUCCESS )
	{
		panic("Unable to create console device");
	}
	
	/*put rootbus specific info to device extension structure*/
	ext = (ROOTBUS_DEVICE_EXTENSION_PTR)console_device_object->device_extension;
	strcpy( ext->name, "console" );
	/*attach the console device to root bus device io stack*/
	AttachDeviceToDeviceStack(console_device_object, pDeviceObject);

	/*put the device object in the device relations structure*/
	dr->count = 2;
	dr->objects[0] = acpi_device_object;
	dr->objects[1] = console_device_object;
	
	return dr;
}
static ERROR_CODE MajorFunctionPnp(DEVICE_OBJECT_PTR pDeviceObject, IRP_PTR pIrp)
{
	ROOTBUS_DEVICE_EXTENSION_PTR ext=NULL;
	IO_STACK_LOCATION_PTR io_stack = GetCurrentIrpStackLocation(pIrp);
	pIrp->io_status.status = ERROR_NOT_SUPPORTED;
	pIrp->io_status.information = NULL;
	switch( io_stack->minor_function )
	{
		case IRP_MN_QUERY_DEVICE_RELATIONS:
			if ( io_stack->parameters.query_device_relations.type == DEVICE_RELATIONS_TYPE_BUS_RELATION )
			{
				/*create real bus drivers only once(local buses cant be unplugged)*/
				if ( pIrp->io_status.information == NULL )
				{
					pIrp->io_status.information = CreateRootBusDevices(pDeviceObject);
				}
				pIrp->io_status.status = ERROR_SUCCESS;
			}
			else
				return ERROR_NOT_FOUND;
			break;
		case IRP_MN_QUERY_ID:
			ext = (ROOTBUS_DEVICE_EXTENSION_PTR)pDeviceObject->device_extension;
			if ( io_stack->parameters.query_id.id_type == BUS_QUERY_DEVICE_ID )
			{
				pIrp->io_status.information = kmalloc( DRIVER_NAME_MAX, 0 );
				if ( pIrp->io_status.information  == NULL )
				{
					pIrp->io_status.status = ERROR_NOT_ENOUGH_MEMORY;
					return ERROR_NOT_ENOUGH_MEMORY;
				}
				strcpy( pIrp->io_status.information, ext->name);
				pIrp->io_status.status = ERROR_SUCCESS;				
			}
			else if ( io_stack->parameters.query_id.id_type == BUS_QUERY_INSTANCE_ID )
			{
				pIrp->io_status.information = NULL;
				pIrp->io_status.status = ERROR_SUCCESS;
			}
			else
				return ERROR_NOT_SUPPORTED;
			break;
	}
	return ERROR_SUCCESS;
}
