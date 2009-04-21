/*!
  \file		drivers/pci/pci.h
  \brief	PCI bus related structures
*/

#ifndef _PCI_H
#define _PCI_H

#include <ace.h>

typedef struct pci_control_register PCI_CONTROL_REGISTER, * PCI_CONTROL_REGISTER_PTR;
typedef struct pci_standard PCI_STANDARD, * PCI_STANDARD_PTR;
typedef struct pci_configuration_space PCI_CONFIGURATION_SPACE, * PCI_CONFIGURATION_SPACE_PTR;
typedef struct pci_status_register PCI_STATUS_REGISTER, * PCI_STATUS_REGISTER_PTR;
typedef struct pci_bist PCI_BIST, * PCI_BIST_PTR;
typedef struct pci_base_address_register PCI_BASE_ADDRESS_REGISTER, * PCI_BASE_ADDRESS_REGISTER_PTR;

/*! pci device command register format*/
struct pci_control_register
{
	union
	{
		UINT16	
			enable_io_space:1,
			enable_memory_space:1,
			enable_master_bus:1,
			enable_special_cycles:1,
			enable_memory_write_and_invalidate:1,
			enable_vga_pallete_snoop:1,
			enable_parity_error:1,
			reserved1:1,
			serr:1,
			enable_fast_back_to_back:1,
			enable_interrupt:1;	
		UINT16 
			all;
	};
};

/*! pci device status register format*/
struct pci_status_register
{
	union
	{
		UINT16 
			reserved1:3,
			interrupt_status:1,
			capability_list:1,
			capable_of_66mhz:1,
			reserved2:1,
			fast_back_to_back:1,
			parity_error:1,
			devsel_timing:2,
			signaled_target_abort:1,
			received_target_abort:1,
			received_master_abort:1,
			signaled_system_error:1,
			detected_parity_error:1;
		UINT16
			all;
		
	};
};
/*Built-in Self Test layout*/
struct pci_bist
{
	BYTE	completion_code:4,
			reserved:2,
			start_bist:1,
			bist_capable:1;
};

struct pci_base_address_register
{
	union
	{
		struct
		{
			UINT32	io_space:1,
					type:2, 	/* 0-32 bit , 2-64bit*/
					prefetchable:1,
					base_address:28;
		}memory;
		struct
		{
			UINT32	io_space:1,
					reserved:1,
					base_address:30;
		}io;
	};
};

/*standard pci bus*/
struct pci_standard
{
	PCI_BASE_ADDRESS_REGISTER 	base_address[6];
	UINT32 						card_bus_cis;
	UINT16						sub_system_vendor_id;
	UINT16						sub_system_device_id;
	UINT32						expansion_rom_base;
	BYTE						reserved1;
	UINT16						reserved2;
	BYTE						capability_pointer;
	UINT32						reserved3;
	BYTE						max_lat;
	BYTE						min_gnt;
	BYTE						interrupt_pin;
	BYTE						interrupt_line;
	UINT32						device_specific[48];
};

/*! PCI Configuration Space*/
struct pci_configuration_space
{
	UINT16					vendor_id;				/*! manufacturer of the device*/
	UINT16					device_id;				/*! This field identifies the particular device.*/
	PCI_CONTROL_REGISTER	control_register;		/*! control register*/
	PCI_STATUS_REGISTER		status_register;		/*! Status register*/
	BYTE					revision_id;			/*! This register specifies a device specific revision identifier*/
	BYTE					programming_interface;	/*! class code - programming interface*/
	BYTE					sub_class_code;			/*! class code - sub class*/
	BYTE					base_class_code;		/*! class code - base class*/
	BYTE					cache_line_size;
	BYTE					latency;
	BYTE					header_type:7,			/*! 0-Standard, 1-PCI Bridge, 2-Cardbus*/
							multifuction_device:1;	/*! if set multi function device else single function*/
	PCI_BIST				bist;					/*! built-in self test*/
	union
	{
		PCI_STANDARD		pci_standard;
	};
};

int get_pci_class_string(BYTE class_code, BYTE subclass_code, BYTE prog_if_code, char ** class_name, char ** subclass_name, char ** prog_if_name );

#endif
