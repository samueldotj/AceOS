/*!
  \file		drivers/acpi.h
  \brief	ACPI related structures and functions
*/

#ifndef _ACPI_H
#define _ACPI_H

#define ACPI_DEVICE_NAME_MAX	50

/*! ACPI device extension*/
struct acpi_bus_device_extension
{
	ACPI_HANDLE	handle;								/*! ACPI Handle of an object*/
	char		device_name[ACPI_DEVICE_NAME_MAX];	/*! full path of an acpi object*/
	int 		pnp_id;
};
typedef struct acpi_bus_device_extension ACPI_BUS_DEVICE_EXTENSION, * ACPI_BUS_DEVICE_EXTENSION_PTR;

/*! argument to AcpiGetDeviceCallback function*/
struct acpi_device_callback_argument
{
	DEVICE_OBJECT_PTR	device_object;				/*! ACPI bus device object*/
	int					total_devices_found;		/*! output - total devices found */
};
typedef struct acpi_device_callback_argument ACPI_DEVICE_CALLBACK_ARGUMENT, * ACPI_DEVICE_CALLBACK_ARGUMENT_PTR;

#endif
