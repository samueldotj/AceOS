/*!
  \file		kernel/iom/iom.h
  \brief	IO Manager
*/

#ifndef _IOM_H_
#define _IOM_H_

#include <ace.h>
#include <ds/avl_tree.h>
#include <sync/spinlock.h>
#include <heap/slab_allocator.h>
#include <kernel/error.h>
#include <kernel/mm/kmem.h>

#define DRIVER_NAME_MAX		50

#define SIZEOF_DEVICE_RELATIONS(number_of_pdos)		(sizeof(DEVICE_RELATIONS) + ((number_of_pdos) * sizeof(DEVICE_OBJECT)) )

typedef enum
{
	IRP_MJ_CREATE	= 0,
	IRP_MJ_CLOSE,
	IRP_MJ_READ,
	IRP_MJ_WRITE,
	IRP_MJ_QUERY_INFORMATION,
	IRP_MJ_SET_INFORMATION,
	IRP_MJ_FLUSH_BUFFERS,
	IRP_MJ_DEVICE_CONTROL,
	IRP_MJ_SHUTDOWN,
	IRP_MJ_LOCK_CONTROL,
	IRP_MJ_CLEANUP,
	IRP_MJ_POWER,
	IRP_MJ_PNP,
	IRP_MJ_MAXIMUM_FUNCTION
}IRP_MJ;

typedef enum
{
	IRP_MN_START_DEVICE,
	IRP_MN_QUERY_REMOVE_DEVICE,
	IRP_MN_REMOVE_DEVICE,
	IRP_MN_CANCEL_REMOVE_DEVICE,
	IRP_MN_STOP_DEVICE,
	IRP_MN_QUERY_STOP_DEVICE,
	IRP_MN_CANCEL_STOP_DEVICE,

	IRP_MN_QUERY_DEVICE_RELATIONS,
	IRP_MN_QUERY_ID,
	IRP_MN_QUERY_PNP_DEVICE_STATE,
	IRP_MN_QUERY_BUS_INFORMATION,
	IRP_MN_QUERY_CAPABILITIES,
	IRP_MN_QUERY_RESOURCES,
	IRP_MN_QUERY_RESOURCE_REQUIREMENTS,
	IRP_MN_QUERY_DEVICE_TEXT,
	IRP_MN_FILTER_RESOURCE_REQUIREMENTS,

	IRP_MN_READ_CONFIG,
	IRP_MN_WRITE_CONFIG,
	IRP_MN_EJECT,
	IRP_MN_SET_LOCK,
	
	IRP_MN_SURPRISE_REMOVAL
}IRP_MN_PNP;

typedef enum
{
	DEVICE_RELATIONS_TYPE_BUS_RELATION
}DEVICE_RELATION_TYPE;

typedef enum
{
	BUS_QUERY_DEVICE_ID, 				/*example for pci device PCI\VEN_xxxx&DEV_xxxx&SUBSYS_xxxxxxxx&REV_xx*/
	BUS_QUERY_INSTANCE_ID,				/*an id which differentiates two same kind of devices on a machine*/
}QUERY_ID_TYPE;

typedef struct driver_object DRIVER_OBJECT, * DRIVER_OBJECT_PTR;
typedef struct device_object DEVICE_OBJECT, * DEVICE_OBJECT_PTR;
typedef struct io_status_block IO_STATUS_BLOCK, * IO_STATUS_BLOCK_PTR;
typedef struct driver_functions DRIVER_FUNCTIONS, * DRIVER_FUNCTIONS_PTR;
typedef struct irp IRP, * IRP_PTR;
typedef struct io_stack_location IO_STACK_LOCATION, *IO_STACK_LOCATION_PTR;
typedef struct device_capabilities DEVICE_CAPABILITIES, *DEVICE_CAPABILITIES_PTR;
typedef struct device_relations DEVICE_RELATIONS, * DEVICE_RELATIONS_PTR;
typedef UINT32 (*IO_COMPLETION_ROUTINE) (DEVICE_OBJECT_PTR device_object, IRP_PTR irp, void * context);

struct driver_functions
{
		ERROR_CODE (*DriverEntry)(DRIVER_OBJECT_PTR pDriverObject);							/*! Called to initialize a driver - called only once*/
		ERROR_CODE (*DriverUnload)(DRIVER_OBJECT_PTR pDriverObject);						/*! Called to destroy driver - called during driver exit*/
		
		ERROR_CODE (*AddDevice)(DRIVER_OBJECT_PTR pDriverObject, DEVICE_OBJECT_PTR pPdo);	/*! When a appropriate device is found this function is called*/
		
		ERROR_CODE (*StartIo)(DEVICE_OBJECT_PTR pDeviceObject, IRP_PTR pIrp);				/*! Called to start IO on a device*/
		ERROR_CODE (*MajorFunctions[IRP_MJ_MAXIMUM_FUNCTION]) (DEVICE_OBJECT_PTR pDeviceObject, IRP_PTR pIrp);		/*! Various functions supported by the device*/
};

struct driver_object
{
	SPIN_LOCK			lock;								/*! For mp sync*/
	int					reference_count;					/*! For garbage collection*/
	LIST				driver_list;						/*! List of loaded drivers in the system*/

	DEVICE_OBJECT_PTR	device_object_head;					/*! Head to list of devices this driver handles*/

	char 				driver_name[DRIVER_NAME_MAX];		/*! Human readable name of the driver*/
	char *				driver_file_name;					/*! File name of the driver - used to find whether a driver is already loaded - once file system is done it should be removed*/
	void *				driver_extension;					/*! Driver specific data*/

	DRIVER_FUNCTIONS 	fn;									/*! All the functions supported by the driver*/
};

struct device_object
{
	SPIN_LOCK			lock;								/*! For mp sync*/
	int					reference_count;					/*! For garbage collection*/
	
	DEVICE_OBJECT_PTR	parent_device;						/*! parent device - may be bus device*/
	LIST				sibilings_list;						/*! list of devices that are created by the same parent*/
	DEVICE_OBJECT_PTR	child_device;						/*! pointer to the first child*/
	
	DRIVER_OBJECT_PTR	driver_object;						/*! Driver of this device*/
	LIST				device_object_list;					/*! List of devices this driver handles*/
	
	BYTE				stack_count;						/*! Minimum IO stack required to pass down*/
	
	void *				device_extension;					/*! Device specific data*/
};

struct io_status_block{
    UINT32				status;								/*! IO completion status*/
    void *				information;						/*! IO completion information (request-dependent value)*/
};

struct irp
{
	UINT32					flags;

	IO_STATUS_BLOCK			io_status;						/*! Status of the IRP*/
	
	BYTE					stack_count;					/*! Total IO_STACK associated with this IRP*/
	BYTE					current_stack;					/*! Current IO_STACK index*/
	
	BYTE					processor_mode;					/*! Processor mode(user/kernel) of the initiator*/
	IO_STACK_LOCATION_PTR	current_stack_location;			/*! Current IO Stack location associated with the given IRP*/
	
};

struct io_stack_location
{
	BYTE				major_function;						/*! Major function code*/
	BYTE				minor_function;						/*! Minor function code*/
	
	UINT32				flag;
	
	union													/*! Various parameters based on major function code*/
	{
		struct
		{
			UINT32		options;
		}create;
		struct
		{
			UINT32		length;
			UINT32		byte_offset;
		}read_write;
		struct
		{
			UINT32		output_buffer_length;
			UINT32		input_buffer_length;
			UINT32		io_control_code;
		}device_io_control;
		struct 
		{
			DEVICE_RELATION_TYPE	type;
		}query_device_relations;
		struct
		{
			QUERY_ID_TYPE			id_type;
		}query_id;
		struct
		{
			DEVICE_CAPABILITIES_PTR device_capabilities;
		}capabilities;
	}parameters;
	
	DEVICE_OBJECT_PTR 	device_object;						/*! Device object associated with this IO stack*/
	
	IO_COMPLETION_ROUTINE completion_routine;				/*! Completion routine registered by the driver*/
	void * context;											/*! Context to passed driver completion routine*/
};

struct device_capabilities{
	UINT32		lock_supported:1;							/*! Whether device supports locking(preventing from ejection)*/
	UINT32		eject_supported:1;							/*! Whether device supports ejection*/
	UINT32		removable:1;								/*! removable device*/
	UINT32		dock_device:1;								/*! dock device*/
	
	UINT32		hardware_disabled:1;						/*! device is disabled in hardware level(present but disabled)*/
	
	UINT32		address;									/*! hardware specific address*/
	
	struct
	{
		UINT32				device_d1:1;					/*! Specifies whether the device hardware supports the D1 power state.*/
		UINT32				device_d2:1;					/*! Specifies whether the device hardware supports the D2 power state.*/
		UINT32				wake_from_d0:1;					/*! Specifies whether the device can respond to an external wake signal while in the D0 state.*/
		UINT32				wake_from_d1:1;					/*! Specifies whether the device can respond to an external wake signal while in the D1 state.*/
		UINT32				wake_from_d2:1;					/*! Specifies whether the device can respond to an external wake signal while in the D2 state.*/
		UINT32				wake_from_d3:1;					/*! Specifies whether the device can respond to an external wake signal while in the D3 state.*/
		UINT32				d1_latency;
		UINT32				d2_latency;
		UINT32				d3_latency;
	}power;
};

struct device_relations{
    UINT32 count;
    DEVICE_OBJECT_PTR objects[0];
};


extern DEVICE_OBJECT_PTR root_bus_device_object;
extern DRIVER_OBJECT_PTR root_bus_driver_object;

extern CACHE driver_object_cache;
extern CACHE device_object_cache;

void InitIoManager();
ERROR_CODE CreateDevice(DRIVER_OBJECT_PTR driver_object, UINT32 device_extension_size, DEVICE_OBJECT_PTR * device_object);
DEVICE_OBJECT_PTR AttachDeviceToDeviceStack(DEVICE_OBJECT_PTR source_device, DEVICE_OBJECT_PTR target_device);
IO_STACK_LOCATION_PTR GetNextIrpStackLocation(IRP_PTR Irp);
IO_STACK_LOCATION_PTR GetCurrentIrpStackLocation(IRP_PTR Irp);
void InvalidateDeviceRelations(DEVICE_OBJECT_PTR device_object, DEVICE_RELATION_TYPE type);
IRP_PTR AllocateIrp(BYTE stack_size);
void ReuseIrp(IRP_PTR irp, ERROR_CODE error_code);
void FreeIrp(IRP_PTR irp);
ERROR_CODE CallDriver(DEVICE_OBJECT_PTR device_object, IRP_PTR irp);
inline void FillIoStack(IO_STACK_LOCATION_PTR io_stack, BYTE major_function, BYTE minor_function, DEVICE_OBJECT_PTR device_object, IO_COMPLETION_ROUTINE completion_routine, void * context);

DRIVER_OBJECT_PTR LoadRootBusDriver();
ERROR_CODE RootBusDriverEntry(DRIVER_OBJECT_PTR pDriverObject);
#endif
