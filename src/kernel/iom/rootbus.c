/*!
	\file	kernel/iom/rootbus.c
	\brief	Imaginary root bus which connects all legacy(ISA) and local bus(PCI) in the system.
*/
#include <ace.h>
#include <stdlib.h>
#include <string.h>
#include <kernel/iom/iom.h>

#define PCI_BUS_NAME	"pci_bus"
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
	return root_bus_driver_object;
}

ERROR_CODE RootBusDriverEntry(DRIVER_OBJECT_PTR pDriverObject)
{
	strcpy( pDriverObject->driver_name, "Root" );
	pDriverObject->driver_extension = NULL;
	pDriverObject->fn.MajorFunctions[IRP_MJ_PNP] = MajorFunctionPnp;

	return ERROR_SUCCESS;
}
static ERROR_CODE MajorFunctionPnp(DEVICE_OBJECT_PTR pDeviceObject, IRP_PTR pIrp)
{
	ROOTBUS_DEVICE_EXTENSION_PTR pci_bus_do_ext=NULL;
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
					DEVICE_RELATIONS_PTR dr;
					DEVICE_OBJECT_PTR pci_bus_do;
					ERROR_CODE err;
					
					/*allocate memory for device relation struction*/
					dr = kmalloc( SIZEOF_DEVICE_RELATIONS(1), 0 );
					if ( dr == NULL )
					{
						pIrp->io_status.status = ERROR_NOT_ENOUGH_MEMORY;
						return ERROR_NOT_ENOUGH_MEMORY;
					}
					/*create device object for pci bus*/
					err = CreateDevice( pDeviceObject->driver_object, sizeof(ROOTBUS_DEVICE_EXTENSION), &pci_bus_do );
					if ( err != ERROR_SUCCESS )
					{
						kfree( dr );
						pIrp->io_status.status = err;
						return err;
					}
					/*put rootbus specific info to device extension structure*/
					pci_bus_do_ext = (ROOTBUS_DEVICE_EXTENSION_PTR)pci_bus_do->device_extension;
					strcpy( pci_bus_do_ext->name, PCI_BUS_NAME );
					/*attach the pci device to root bus device io stack*/
					AttachDeviceToDeviceStack(pci_bus_do, pDeviceObject);
					
					/*put the device object in the device relations structure*/
					dr->count = 1;
					dr->objects[0] = pci_bus_do;
					pIrp->io_status.information = dr;
				}
				pIrp->io_status.status = ERROR_SUCCESS;
			}
			else
				return ERROR_NOT_FOUND;
			break;
		case IRP_MN_QUERY_ID:
			pci_bus_do_ext = (ROOTBUS_DEVICE_EXTENSION_PTR)pDeviceObject->device_extension;
			if ( io_stack->parameters.query_id.id_type == BUS_QUERY_DEVICE_ID )
			{
				pIrp->io_status.information = kmalloc( DRIVER_NAME_MAX, 0 );
				if ( pIrp->io_status.information  == NULL )
				{
					pIrp->io_status.status = ERROR_NOT_ENOUGH_MEMORY;
					return ERROR_NOT_ENOUGH_MEMORY;
				}
				strcpy( pIrp->io_status.information, pci_bus_do_ext->name);
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
