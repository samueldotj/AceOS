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

static ERROR_CODE MajorFunctionPnp(DEVICE_OBJECT_PTR device_object, IRP_PTR irp);
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
	err = CreateDevice(pDriverObject, sizeof(ACPI_BUS_DEVICE_EXTENSION), &device_object, "ACPI", DO_BUFFERED_IO);
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
	ACPI_DEVICE_INFO * dev_info_ptr;
	ACPI_STATUS ret;
	
	arg = (ACPI_DEVICE_CALLBACK_ARGUMENT_PTR) Context;
	
	/*for debugging purpose trace the name of the device*/
	buf.Pointer = name;
	buf.Length = sizeof(name);
	if ( AcpiGetName(ObjHandle, ACPI_FULL_PATHNAME, &buf) == AE_OK )
		ktrace("Found device ACPI %s ", name );	
	
	/*get the required size*/
	buf.Pointer = NULL;
	buf.Length = ACPI_ALLOCATE_BUFFER;
	ret = AcpiGetObjectInfo(ObjHandle, &buf); 
	
	if ( ret != AE_OK )
		return AE_OK;
	
	/*get hardware id (HID)*/
	dev_info_ptr = kmalloc(buf.Length, KMEM_NO_FAIL); 
	buf.Pointer = dev_info_ptr;
	ret = AcpiGetObjectInfo(ObjHandle, &buf);
	/*if HID is reterived, use it to create a new device*/
	if ( ret == AE_OK && dev_info_ptr->Valid & ACPI_VALID_HID )
	{
		DEVICE_OBJECT_PTR child_device_object;
		if ( CreateDevice(arg->device_object->driver_object, sizeof(ACPI_BUS_DEVICE_EXTENSION), &child_device_object, NULL, DO_BUFFERED_IO) == ERROR_SUCCESS )
		{
			ACPI_BUS_DEVICE_EXTENSION_PTR device_ext=child_device_object->device_extension;
			device_ext->handle = ObjHandle;
			memcpy(device_ext->device_name, dev_info_ptr->HardwareId.Value, ACPI_DEVICE_ID_LENGTH );
			AttachDeviceToDeviceStack(child_device_object, arg->device_object);
			arg->total_devices_found ++;
		}
		ktrace("%s \n", dev_info_ptr->HardwareId.Value);
	}
	else
		ktrace("AcpiGetObjectInfo ret %d valid bits %d\n", ret, dev_info_ptr->Valid );
	kfree(dev_info_ptr);
	
	return AE_OK;
}
static ERROR_CODE MajorFunctionPnp(DEVICE_OBJECT_PTR device_object, IRP_PTR irp)
{
	ACPI_BUS_DEVICE_EXTENSION_PTR acpi_dev_ext=NULL;
	IO_STACK_LOCATION_PTR io_stack = GetCurrentIrpStackLocation(irp);
	void * return_value;
	
	irp->io_status.status = ERROR_NOT_SUPPORTED;
	irp->io_status.information = NULL;
	switch( io_stack->minor_function )
	{
		case IRP_MN_QUERY_DEVICE_RELATIONS:
			if ( io_stack->parameters.query_device_relations.type == DEVICE_RELATIONS_TYPE_BUS_RELATION )
			{
				int i=0;
				DEVICE_RELATIONS_PTR dr;
				ACPI_DEVICE_CALLBACK_ARGUMENT arg;
				
				arg.total_devices_found = 0;
				arg.device_object = device_object;
				
				AcpiGetDevices(NULL, AcpiGetDeviceCallback, &arg, &return_value);
				/*put the new device relations list*/
				if ( arg.total_devices_found > 0 )
				{
					LIST_PTR cur_node;
					/*allocate memory for device relation struction*/
					dr = kmalloc( SIZEOF_DEVICE_RELATIONS(arg.total_devices_found), 0 );
					if ( dr == NULL )
					{
						irp->io_status.status = ERROR_NOT_ENOUGH_MEMORY;
						return ERROR_NOT_ENOUGH_MEMORY;
					}
					dr->count = arg.total_devices_found;
					assert( device_object->driver_object->device_object_head );
					dr->objects[0] = device_object->driver_object->device_object_head;
					i=1;
					LIST_FOR_EACH(cur_node, &device_object->driver_object->device_object_head->device_object_list )
					{
						dr->objects[i] = STRUCT_ADDRESS_FROM_MEMBER(cur_node, DEVICE_OBJECT, device_object_list);
						i++;
					}
					
					/*return success*/
					irp->io_status.information = dr;
					irp->io_status.status = ERROR_SUCCESS;
					return ERROR_SUCCESS;
				}
			}
			
			break;
		case IRP_MN_QUERY_ID:
			acpi_dev_ext = (ACPI_BUS_DEVICE_EXTENSION_PTR)device_object->device_extension;
			assert( acpi_dev_ext != NULL );
			irp->io_status.information = NULL;
			if ( io_stack->parameters.query_id.id_type == BUS_QUERY_DEVICE_ID )
			{
				irp->io_status.information = kmalloc( DRIVER_NAME_MAX, 0 );
				if ( irp->io_status.information  == NULL )
				{
					irp->io_status.status = ERROR_NOT_ENOUGH_MEMORY;
					return ERROR_NOT_ENOUGH_MEMORY;
				}
				strcpy(irp->io_status.information, acpi_dev_ext->device_name);
				irp->io_status.status = ERROR_SUCCESS;				
			}
			else if ( io_stack->parameters.query_id.id_type == BUS_QUERY_INSTANCE_ID )
			{
				irp->io_status.status = ERROR_SUCCESS;
			}
			else
				return ERROR_NOT_SUPPORTED;
			break;
	}
	return ERROR_SUCCESS;
}
