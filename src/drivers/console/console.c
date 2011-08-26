/*!
	\file	drivers/console/console.c
	\brief	i386 console driver - keyboard and text mode video 
*/
#include <ace.h>
#include <string.h>
#include <kernel/io.h>
#include <kernel/error.h>
#include <kernel/iom/iom.h>
#include <kernel/interrupt.h>
#include <kernel/debug.h>


static ERROR_CODE AddDevice(DRIVER_OBJECT_PTR drv_obj, DEVICE_OBJECT_PTR parent_dev_obj);
static ERROR_CODE MajorFunctionPnP(DEVICE_OBJECT_PTR dev_obj, IRP_PTR irp);
static ERROR_CODE MajorFunctionRead(DEVICE_OBJECT_PTR dev_obj, IRP_PTR irp);
static ERROR_CODE MajorFunctionWrite(DEVICE_OBJECT_PTR dev_obj, IRP_PTR irp);

/*!	Entry point - called during driver initialization
 * \param	drv_obj	Driver object pointer which should be initialised
 * Returns standard error code.
 */
ERROR_CODE DriverEntry(DRIVER_OBJECT_PTR drv_obj)
{
	KPRINTF("called");
	strcpy( drv_obj->driver_name, "console" );
	drv_obj->driver_extension = NULL;
	drv_obj->fn.AddDevice = AddDevice;
	drv_obj->fn.MajorFunctions[IRP_MJ_READ] = MajorFunctionRead;
	drv_obj->fn.MajorFunctions[IRP_MJ_WRITE] = MajorFunctionWrite;
	drv_obj->fn.MajorFunctions[IRP_MJ_PNP] = MajorFunctionPnP;
	
	KTRACE("called");

	return ERROR_SUCCESS;
}

/*!
 * \brief	Adds a new device to the device hierarchy and binds the device with the given driver.
 * \param	drv_obj	Driver object to which the new device must be associated.
 * \param	parent_dev_obj	Device object's parent pointer.
 * Returns a standard error code
 */
static ERROR_CODE AddDevice(DRIVER_OBJECT_PTR drv_obj, DEVICE_OBJECT_PTR parent_dev_obj)
{
	DEVICE_OBJECT_PTR dev_obj;
	ERROR_CODE err;

	err = CreateDevice(drv_obj, 0, &dev_obj, "Console", DO_BUFFERED_IO);
	KTRACE("console device object %p\n", dev_obj);
	if( err != ERROR_SUCCESS )
		return err;
		
	/* Now establish the hierarchy between parent_dev_obj and dev_obj */
	(void)AttachDeviceToDeviceStack(dev_obj, parent_dev_obj);

	return ERROR_SUCCESS;
}

/*!	Function to handle all Plug and play activities associated with keyboard.
 * \param	dev_obj	Device for which request is made.
 * \param	irp		Interrupt request packet pointer containing user request details
 * Returns SUCCESS if request can be processed or else suitable failure code is returned.
 */
static ERROR_CODE MajorFunctionPnP(DEVICE_OBJECT_PTR dev_obj, IRP_PTR irp)
{
	KTRACE("PNP func called");
	return ERROR_SUCCESS;
}

static ERROR_CODE MajorFunctionRead(DEVICE_OBJECT_PTR dev_obj, IRP_PTR irp)
{
	KTRACE("Read called");
	return ERROR_SUCCESS;
}
static ERROR_CODE MajorFunctionWrite(DEVICE_OBJECT_PTR dev_obj, IRP_PTR irp)
{
	KTRACE("Write called");
	return ERROR_SUCCESS;
}
	
