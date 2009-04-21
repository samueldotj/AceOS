/*!
	\file	drivers/pci/pci.c
	\brief	PCI bus driver
*/
#include <ace.h>
#include <stdlib.h>
#include <string.h>
#include <kernel/io.h>
#include <kernel/debug.h>
#include <kernel/iom/iom.h>
#include <kernel/acpi/acpi.h>
#include "pci.h"


#define	MAX_PCI_BUS					0xFF
#define	MAX_PCI_DEVICE_PER_BUS		32
#define	MAX_PCI_FUNCTION_PER_DEVICE	8

#if ARCH==i386
	#define PCI_CONFIG_ADDRESS	0xCF8
	#define PCI_CONFIG_DATA		0xCFC
#endif

typedef struct pci_bus_device_extension PCI_BUS_DEVICE_EXTENSION, * PCI_BUS_DEVICE_EXTENSION_PTR;

struct pci_bus_device_extension
{
	PCI_CONFIGURATION_SPACE pci_conf;
};

static ERROR_CODE MajorFunctionPnp(DEVICE_OBJECT_PTR pDeviceObject, IRP_PTR pIrp);
static ERROR_CODE AddDevice(DRIVER_OBJECT_PTR pDriverObject, DEVICE_OBJECT_PTR pPdo);

static inline unsigned char pci_read_byte(int bus_no, int device_no, int function_no, int offset)
{
  _outpd(PCI_CONFIG_ADDRESS, ((unsigned long) 0x80000000 | (bus_no << 16) | (device_no << 11) | (function_no << 8) | offset));
  return _inp(PCI_CONFIG_DATA);
}

static inline unsigned short pci_read_word(int bus_no, int device_no, int function_no, int offset)
{
  _outpd(PCI_CONFIG_ADDRESS, ((unsigned long) 0x80000000 | (bus_no << 16) | (device_no << 11) | (function_no << 8) | offset));
  return _inpw(PCI_CONFIG_DATA);
}

static inline unsigned long pci_read_dword(int bus_no, int device_no, int function_no, int offset)
{
  _outpd(PCI_CONFIG_ADDRESS, ((unsigned long) 0x80000000 | (bus_no << 16) | (device_no << 11) | (function_no << 8) | offset));
  return _inpd(PCI_CONFIG_DATA);
}

static inline void pci_write_byte(int bus_no, int device_no, int function_no, int offset, unsigned char value)
{
  _outpd(PCI_CONFIG_ADDRESS, ((unsigned long) 0x80000000 | (bus_no << 16) | (device_no << 11) | (function_no << 8) | offset));
  _outp(PCI_CONFIG_DATA, value);
}

static inline void pci_write_word(int bus_no, int device_no, int function_no, int offset, unsigned short value)
{
  _outpd(PCI_CONFIG_ADDRESS, ((unsigned long) 0x80000000 | (bus_no << 16) | (device_no << 11) | (function_no << 8) | offset));
  _outpw(PCI_CONFIG_DATA, value);
}

static inline void pci_write_dword(int bus_no, int device_no, int function_no, int offset, unsigned long value)
{
  _outpd(PCI_CONFIG_ADDRESS, ((unsigned long) 0x80000000 | (bus_no << 16) | (device_no << 11) | (function_no << 8) | offset));
  _outpd(PCI_CONFIG_DATA, value);
}

/*! Reads configuration space of the specified bus, device and function.
	\return 0 - on success and pci_conf will be updated with correct values
	\return 1 - on failure and pci_conf is not affected
*/
static int read_pci_configuration_space(BYTE bus_no, BYTE device_no, BYTE function_no, PCI_CONFIGURATION_SPACE_PTR pci_conf)
{
	int offset;
	UINT32 * buffer = (UINT32 *)pci_conf;
	buffer[0] = pci_read_dword(bus_no, device_no, function_no, 0);
	if ( pci_conf->vendor_id == 0 || pci_conf->vendor_id == 0xFFFF )
		return 1;
		
	for(offset=1; offset<(sizeof(PCI_CONFIGURATION_SPACE)>>2); offset++ )
		buffer[offset] = pci_read_dword(bus_no, device_no, function_no, offset<<2);
	return 0;
}

/*Entry point - called during driver initialization*/
ERROR_CODE DriverEntry(DRIVER_OBJECT_PTR pDriverObject)
{
	strcpy( pDriverObject->driver_name, "PCI Bus" );
	pDriverObject->driver_extension = NULL;
	pDriverObject->fn.AddDevice = AddDevice;
	pDriverObject->fn.MajorFunctions[IRP_MJ_PNP] = MajorFunctionPnp;
	        
	return ERROR_SUCCESS;
}
static ERROR_CODE AddDevice(DRIVER_OBJECT_PTR pDriverObject, DEVICE_OBJECT_PTR pPdo)
{
	DEVICE_OBJECT_PTR device_object;
	ERROR_CODE err;
	err = CreateDevice(pDriverObject, sizeof(PCI_BUS_DEVICE_EXTENSION), &device_object);
	if( err != ERROR_SUCCESS )
		return err;
	InvalidateDeviceRelations(device_object, DEVICE_RELATIONS_TYPE_BUS_RELATION);
	
	return ERROR_SUCCESS;
}
static ERROR_CODE MajorFunctionPnp(DEVICE_OBJECT_PTR pDeviceObject, IRP_PTR pIrp)
{
	int bus_no, device_no, function_no;
	PCI_BUS_DEVICE_EXTENSION_PTR pci_dev_ext=NULL;
	IO_STACK_LOCATION_PTR io_stack = GetCurrentIrpStackLocation(pIrp);
	
	pIrp->io_status.status = ERROR_NOT_SUPPORTED;
	pIrp->io_status.information = NULL;
	switch( io_stack->minor_function )
	{
		case IRP_MN_QUERY_DEVICE_RELATIONS:
			if ( io_stack->parameters.query_device_relations.type == DEVICE_RELATIONS_TYPE_BUS_RELATION )
			{
				int i=0, total_devices_found = 0;
				DEVICE_RELATIONS_PTR dr;
				/*scan pci bus find all the devices attached*/
				for (bus_no=0; bus_no<MAX_PCI_BUS ; bus_no++)
				{
					for(device_no=0; device_no<MAX_PCI_DEVICE_PER_BUS; device_no++)
					{
						for(function_no=0; function_no<MAX_PCI_FUNCTION_PER_DEVICE; function_no++)
						{
							PCI_CONFIGURATION_SPACE pci_conf;
							if ( read_pci_configuration_space(bus_no, device_no, function_no, &pci_conf) == 0)
							{
								DEVICE_OBJECT_PTR child_device_object;
								char *class_name, *subclass_name, *prog_if_name;
								get_pci_class_string(pci_conf.base_class_code, pci_conf.sub_class_code, pci_conf.programming_interface, &class_name, &subclass_name, &prog_if_name );
								//kprintf("%x %x %s %s %s\n", pci_conf.vendor_id, pci_conf.device_id,  class_name, subclass_name, prog_if_name );
								if ( CreateDevice(pDeviceObject->driver_object, sizeof(PCI_BUS_DEVICE_EXTENSION), &child_device_object) == ERROR_SUCCESS )
								{
									PCI_BUS_DEVICE_EXTENSION_PTR device_ext=child_device_object->device_extension;
									memcpy(&device_ext->pci_conf, &pci_conf, sizeof(PCI_CONFIGURATION_SPACE) );
									AttachDeviceToDeviceStack(child_device_object, pDeviceObject);
									total_devices_found ++;
								}
							}
						}
					}
				}
	
				/*put the new device relations list*/
				if ( total_devices_found > 0 )
				{
					LIST_PTR cur_node;
					/*allocate memory for device relation struction*/
					dr = kmalloc( SIZEOF_DEVICE_RELATIONS(total_devices_found), 0 );
					if ( dr == NULL )
					{
						pIrp->io_status.status = ERROR_NOT_ENOUGH_MEMORY;
						return ERROR_NOT_ENOUGH_MEMORY;
					}
					dr->count = total_devices_found;
					assert( pDeviceObject->driver_object->device_object_head );
					dr->objects[0] = pDeviceObject->driver_object->device_object_head;
					i=0;
					LIST_FOR_EACH(cur_node, &pDeviceObject->driver_object->device_object_head->device_object_list )
					{
						assert( i<=total_devices_found );
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
			pci_dev_ext = (PCI_BUS_DEVICE_EXTENSION_PTR)pDeviceObject->device_extension;
			if ( io_stack->parameters.query_id.id_type == BUS_QUERY_DEVICE_ID )
			{
				pIrp->io_status.information = kmalloc( DRIVER_NAME_MAX, 0 );
				if ( pIrp->io_status.information  == NULL )
				{
					pIrp->io_status.status = ERROR_NOT_ENOUGH_MEMORY;
					return ERROR_NOT_ENOUGH_MEMORY;
				}
				sprintf(pIrp->io_status.information, "PCI_VEN_%x&DEV_%x&SUBSYS_%d&REV_%d", 
						pci_dev_ext->pci_conf.vendor_id,  pci_dev_ext->pci_conf.device_id, pci_dev_ext->pci_conf.pci_standard.sub_system_device_id, pci_dev_ext->pci_conf.revision_id);
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
