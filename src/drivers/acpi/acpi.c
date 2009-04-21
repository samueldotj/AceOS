/*!
	\file	drivers/acpi/acpi.c
	\brief	ACPI driver
*/
#include <ace.h>
#include <stdlib.h>
#include <string.h>
#include <kernel/io.h>
#include <kernel/debug.h>
#include <kernel/iom/iom.h>
#include <kernel/acpi/acpi.h>
#include "acpi.h"

static ERROR_CODE MajorFunctionPnp(DEVICE_OBJECT_PTR pDeviceObject, IRP_PTR pIrp);
static ERROR_CODE AddDevice(DRIVER_OBJECT_PTR pDriverObject, DEVICE_OBJECT_PTR pPdo);

/*Entry point - called during driver initialization*/
ERROR_CODE DriverEntry(DRIVER_OBJECT_PTR pDriverObject)
{
	strcpy( pDriverObject->driver_name, "ACPI" );
	pDriverObject->driver_extension = NULL;
	pDriverObject->fn.AddDevice = AddDevice;
	pDriverObject->fn.MajorFunctions[IRP_MJ_PNP] = MajorFunctionPnp;
	
	return ERROR_SUCCESS;
}
static ERROR_CODE AddDevice(DRIVER_OBJECT_PTR pDriverObject, DEVICE_OBJECT_PTR pPdo)
{
	DEVICE_OBJECT_PTR device_object;
	ERROR_CODE err;
	err = CreateDevice(pDriverObject, sizeof(ACPI_BUS_DEVICE_EXTENSION), &device_object);
	if( err != ERROR_SUCCESS )
		return err;
	InvalidateDeviceRelations(device_object, DEVICE_RELATIONS_TYPE_BUS_RELATION);
	
	return ERROR_SUCCESS;
}
ACPI_STATUS AcpiGetDeviceCallback(ACPI_HANDLE ObjHandle, UINT32 NestingLevel, void *Context, void **ReturnValue)
{
	ACPI_BUFFER buf;
	char name[ACPI_DEVICE_NAME_MAX];
	ACPI_DEVICE_CALLBACK_ARGUMENT_PTR arg;
	
	buf.Pointer = name;
	buf.Length = sizeof(name);
	
	arg = (ACPI_DEVICE_CALLBACK_ARGUMENT_PTR) Context;
	if ( AcpiGetName(ObjHandle, ACPI_FULL_PATHNAME, &buf) == AE_OK )
	{
		DEVICE_OBJECT_PTR child_device_object;
		if ( CreateDevice(arg->device_object->driver_object, sizeof(ACPI_BUS_DEVICE_EXTENSION), &child_device_object) == ERROR_SUCCESS )
		{
			ACPI_BUS_DEVICE_EXTENSION_PTR device_ext=child_device_object->device_extension;
			device_ext->handle = ObjHandle;
			memcpy(device_ext->device_name, buf.Pointer, ACPI_DEVICE_NAME_MAX );
			AttachDeviceToDeviceStack(child_device_object, arg->device_object);
			arg->total_devices_found ++;
		}
	}

	return AE_OK;
}
static ERROR_CODE MajorFunctionPnp(DEVICE_OBJECT_PTR pDeviceObject, IRP_PTR pIrp)
{
	ACPI_BUS_DEVICE_EXTENSION_PTR acpi_dev_ext=NULL;
	IO_STACK_LOCATION_PTR io_stack = GetCurrentIrpStackLocation(pIrp);
	void * return_value;
	
	pIrp->io_status.status = ERROR_NOT_SUPPORTED;
	pIrp->io_status.information = NULL;
	switch( io_stack->minor_function )
	{
		case IRP_MN_QUERY_DEVICE_RELATIONS:
			if ( io_stack->parameters.query_device_relations.type == DEVICE_RELATIONS_TYPE_BUS_RELATION )
			{
				int i=0;
				DEVICE_RELATIONS_PTR dr;
				ACPI_DEVICE_CALLBACK_ARGUMENT arg;
				
				arg.total_devices_found = 0;
				arg.device_object = pDeviceObject;
				
				AcpiGetDevices(NULL, AcpiGetDeviceCallback, &arg, &return_value);
				/*put the new device relations list*/
				if ( arg.total_devices_found > 0 )
				{
					LIST_PTR cur_node;
					/*allocate memory for device relation struction*/
					dr = kmalloc( SIZEOF_DEVICE_RELATIONS(arg.total_devices_found), 0 );
					if ( dr == NULL )
					{
						pIrp->io_status.status = ERROR_NOT_ENOUGH_MEMORY;
						return ERROR_NOT_ENOUGH_MEMORY;
					}
					dr->count = arg.total_devices_found;
					assert( pDeviceObject->driver_object->device_object_head );
					dr->objects[0] = pDeviceObject->driver_object->device_object_head;
					i=0;
					LIST_FOR_EACH(cur_node, &pDeviceObject->driver_object->device_object_head->device_object_list )
					{
						dr->objects[i] = STRUCT_ADDRESS_FROM_MEMBER(cur_node, DEVICE_OBJECT, device_object_list);
						i++;
					}
					
					/*return success*/
					pIrp->io_status.information = dr;
					pIrp->io_status.status = ERROR_SUCCESS;
					return ERROR_SUCCESS;
				}
			}
			
			break;
		case IRP_MN_QUERY_ID:
			acpi_dev_ext = (ACPI_BUS_DEVICE_EXTENSION_PTR)pDeviceObject->device_extension;
			assert( acpi_dev_ext != NULL );
			if ( io_stack->parameters.query_id.id_type == BUS_QUERY_DEVICE_ID )
			{
				pIrp->io_status.information = kmalloc( DRIVER_NAME_MAX, 0 );
				if ( pIrp->io_status.information  == NULL )
				{
					pIrp->io_status.status = ERROR_NOT_ENOUGH_MEMORY;
					return ERROR_NOT_ENOUGH_MEMORY;
				}
				sprintf(pIrp->io_status.information, "ACPI/%s", &acpi_dev_ext->device_name[1]);
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
