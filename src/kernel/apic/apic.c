/*!
  \file		apic.c
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created:	Sat Jun 14, 2008  06:16PM
  			Last modified: Mon Jun 16, 2008  03:46PM
  \brief	Provides support for Advanced programmable interrupt controller.
*/

#include <ace.h>
#include <kernel/cpuid.h>
#include <string.h>

#define X2APIC_ENABLE_BIT 21
#define APIC_ENABLE_BIT 9
#define LVT_VERSION_REGISTER_OFFSET 			0x30
#define LOGICAL_DESTINATION_REGISTER_OFFSET		0xD0
#define DESTINATION_FORMAT_REGISTER_OFFSET		0xE0

/* APIC base register(machine specific) */
typedef struct ia32_apic_base_msr
{
	UINT32 reserved1: 8; //0-7
	UINT32 bsp: 1; //8th bit-> Is this a bootstrap processor
	UINT32 reserved2: 2; //9-10
	UINT32 enable: 1; //11
	UINT32 base: 24; //12-35
	UINT32 reserved3: 28; //36-63
} IA32_APIC_BASE_MSR, * IA32_APIC_BASE_MSR_PTR;

IA32_APIC_BASE_MSR_PTR ia32_apic_base_msr;
IA32_APIC_BASE_MSR_PTR ia32_ioapic_base_msr;
#define IA32_APIC_BASE_MSR_START 0xfee00000
#define IA32_IOAPIC_BASE_MSR_START 0xfec00000



typedef struct interrupt_command_register
{
	UINT32 vector: 8; //0-7
	UINT32 delivery_mode: 3; //8-10
	UINT32 destination_mode: 1; //11
	UINT32 delivery_status: 1; //12
	UINT32 reserved1: 1; //13
	UINT32 level: 1; //14
	UINT32 trigger_mode: 1; //15
	UINT32 reserved2: 2; //16-17
	UINT32 destination_shorthand: 2; //18-19
	UINT32 reserved3: 12; //20-31
	UINT32 reserved4: 24; //32-55
	UINT32 destination_field: 8; //56-63
}INTERRUPT_COMMAND_REGISTER, * INTERRUPT_COMMAND_REGISTER_PTR;

INTERRUPT_COMMAND_REGISTER_PTR interrupt_command_register;
#define INTERRUPT_COMMAND_REGISTER_LOW_OFFSET	0x300 /*(0-31 bits)*/
#define INTERRUPT_COMMAND_REGISTER_HIGH_OFFSET	0x310 /*(32-63 bits)*/

enum ICR_DELIVERY_MODE
{
    FIXED=0,
    LOWEST_PRIORITY=1,
    SMI=2,
    RESERVED1=3,
    NMI=4,
    INIT=5,
    START_UP=6,
    RESERVED2=7
};

enum ICR_DESTINATION_MODE
{
	PHYSICAL=0,
	LOGICAL=1
};

enum ICR_DELIVERY_STATUS
{
	IDLE=0,
	SEND_PENDING=1
};

enum ICR_LEVEL
{
	DE_ASSERT=0,
	ASSERT=1
};

enum ICR_TRIGGER_MODE
{
	EDGE=0,
	LEVEL=1
};

enum ICR_DESTINATION_SHORTHAND
{
	NO_SHORTHAND=0,
	SELF=1,
	ALL_INCLUDING_SELF=2,
	ALL_EXCLUDING_SELF=3
};


/* Start of Local Vector Table */

typedef struct timer_register
{
	UINT32 vector: 8; //0-7
	UINT32 reserved1: 4; //8-11
	UINT32 delivery_status: 1; //12
	UINT32 reserved2: 3; //13-15
	UINT32 mask: 1; //16
	UINT32 timer_mode: 1; //17
	UINT32 reserved3: 14;//18-31
}TIMER_REGISTER, * TIMER_REGISTER_PTR;

TIMER_REGISTER_PTR timer_register;
#define TIMER_REGISTER_OFFSET 0x320
#define TIMER_REGISTER_RESET 0x00010000

typedef struct lint0_reg
{
	UINT32 vector: 8; //0-7
	UINT32 delivery_mode: 3; //8-10
	UINT32 reserved1: 1; //11
	UINT32 delivery_status: 1; //12
	UINT32 interrupt_input_pin_polarity: 1; //13
	UINT32 remote_irr: 1; //14
	UINT32 trigger_mode: 1; //15
	UINT32 mask: 1; //16
	UINT32 reserved2: 15;//17-31
}LINT0_REG, * LINT0_REG_PTR;

LINT0_REG_PTR lint0_reg;
#define LINT0_REGISTER_OFFSET 0X350
#define LINT0_REGISTER_RESET 0x00010000


typedef struct lint1_reg
{
	UINT32 vector: 8; //0-7
	UINT32 delivery_mode: 3; //8-10
	UINT32 reserved1: 1; //11
	UINT32 delivery_status: 1; //12
	UINT32 interrupt_input_pin_polarity: 1; //13
	UINT32 remote_irr: 1; //14
	UINT32 trigger_mode: 1; //15
	UINT32 mask: 1; //16
	UINT32 reserved2: 15;//17-31
}LINT1_REG, * LINT1_REG_PTR;

LINT1_REG_PTR lint1_reg;
#define LINT1_REGISTER_OFFSET 0X360
#define LINT0_REGISTER_RESET 0x00010000


typedef struct error_reg
{
	UINT32 vector: 8; //0-7
	UINT32 reserved1: 4; //8-11
	UINT32 delivery_status: 1; //12
	UINT32 reserved2: 3;//13-15
	UINT32 mask: 1; //16
	UINT32 reserved3: 15;//17-31
}ERROR_REG, * ERROR_REG_PTR;

ERROR_REG_PTR error_reg;
#define ERROR_REGISTER_OFFSET 0X370
#define ERROR_REGISTER_RESET 0x00010000



typedef struct performance_monitor_count_reg
{
	UINT32 vector: 8; //0-7
	UINT32 delivery_mode: 3; //8-10
	UINT32 reserved1: 1; //11
	UINT32 delivery_status: 1; //12
	UINT32 reserved2: 3;//13-15
	UINT32 mask: 1; //16
	UINT32 reserved3: 15;//17-31
}PERFORMANCE_MONITOR_COUNT_REG, * PERFORMANCE_MONITOR_COUNT_REG_PTR;

PERFORMANCE_MONITOR_COUNT_REG_PTR performance_monitor_count_reg;
#define PERF_MON_CNT_REGISTER_OFFSET 0X340
#define PERF_MON_CNT_REGISTER_RESET 0x00010000 




typedef struct thermal_sensor_reg
{
	UINT32 vector: 8; //0-7
	UINT32 delivery_mode: 3; //8-10
	UINT32 reserved1: 1; //11
	UINT32 delivery_status: 1; //12
	UINT32 reserved2: 3;//13-15
	UINT32 mask: 1; //16
	UINT32 reserved3: 15;//17-31
}THERMAL_SENSOR_REG, * THERMAL_SENSOR_REG_PTR;

THERMAL_SENSOR_REG_PTR thermal_sensor_reg;
#define THERMAL_SENSOR_REGISTER_OFFSET 0x330
#define THERMAL_SENSOR_REGISTER_RESET 0x00010000

/* END OF Local Vector Table */



typedef struct error_status_reg
{
	UINT32 send_checksum_error: 1; //0
	UINT32 receive_checksum_error: 1; //1
	UINT32 send_accept_error: 1; //2
	UINT32 receive_accept_error: 1; //3
	UINT32 reserved1: 1; //4
	UINT32 send_illegal_vector: 1; //5
	UINT32 received_illegal_vector: 1; //6
//#ifdef ( (PENTIUM4) || (INTEL_XEON) || (P6_FAMILY) )
	UINT32 illegal_register_address: 1; //7
//#elif PENTIUM
	UINT32 reserved0 : 1; //7
//#endif
	UINT32 reserved2: 24; //8-31
}ERROR_STATUS_REG, * ERROR_STATUS_REG_PTR;

ERROR_STATUS_REG_PTR error_status_reg;
#define ERROR_STATUS_REGISTER_OFFSET 0x280
#define ERROR_STATUS_REGISTER_RESET 0x0



typedef struct local_apic_version_reg
{
	UINT32 version: 8; //0-7
	UINT32 reserved1: 8; //8-15
	UINT32 max_lvt_entry: 8; //16-23
	UINT32 reserved2: 8; //24-31
}LOCAL_APIC_VERSION_REG, * LOCAL_APIC_VERSION_REG_PTR;

LOCAL_APIC_VERSION_REG_PTR local_apic_version_reg;
#define LOCAl_APIC_VERSION_REGISTER_OFFSET 0x30



typedef struct logical_destination_reg
{
	UINT32 reserved1: 24; //0-23
	UINT32 logical_apic_id: 8; //24-31
}LOGICAL_DESTINATION_REG, * LOGICAL_DESTINATION_REG_PTR;

LOGICAL_DESTINATION_REG_PTR logical_destination_reg;
#define LOGICAL_DESTINATION_REGISTER_OFFSET 0xD0
#define LOGICAL_DESTINATION_REGISTER_RESET 0x0




typedef struct destination_format_reg
{
	UINT32 reserved1: 28; //0-27 all 1's
	UINT32 model: 4; //28-31
}DESTINATION_FORMAT_REG, * DESTINATION_FORMAT_REG_PTR;

DESTINATION_FORMAT_REG_PTR destination_format_reg;
#define DESTINATION_FORMAT_REGISTER_OFFSET 0xE0
#define DESTINATION_FORMAT_REGISTER_RESET 0xFFFFFFFF


enum dfr_model
{
	FLAT_MODEL=15,
	CLUSTER_MODEL=0
};




typedef struct arbitration_priority_reg
{
	UINT32 arbitration_priority_subclass: 4; //0-3
	UINT32 arbitration_priority: 4; //4-7
	UINT32 reserved: 24; //8-31
}ARBITRATION_PRIORITY_REG, * ARBITRATION_PRIORITY_REG_PTR;

ARBITRATION_PRIORITY_REG_PTR arbitration_priority_reg;
#define ARBITRATION_PRIORITY_REGISTER_OFFSET 0x90
#define ARBITRATION_PRIORITY_REGISTER_RESET 0x0




typedef struct task_priority_reg
{
	UINT32 task_priority_subclass: 4; //0-3
	UINT32 task_priority: 4; //4-7
	UINT32 reserved: 24; //8-31
}TASK_PRIORITY_REG, *TASK_PRIORITY_REG_PTR;

TASK_PRIORITY_REG_PTR task_priority_reg;
#define TASK_PRIORITY_REGISTER_OFFSET 0x80
#define TASK_PRIORITY_REGISTER_RESET 0x0




typedef struct processor_priority_reg
{
	UINT32 processor_priority_subclass: 4; //0-3
	UINT32 processor_priority: 4; //4-7
	UINT32 reserved: 24; //8-31
}PROCESSOR_PRIORITY_REG, *PROCESSOR_PRIORITY_REG_PTR;

PROCESSOR_PRIORITY_REG_PTR processor_priority_reg;
#define PROCESSOR_PRIORITY_REGISTER_OFFSET 0xA0
#define PROCESSOR_PRIORITY_REGISTER_RESET 0x0




typedef union interrupt_request_reg
{
	UINT32 vectors[8]; //16-255  32*8 = 256
	UINT32 reserved: 16; //0-15
}INTERRUPT_REQUEST_REG, * INTERRUPT_REQUEST_REG_PTR;

INTERRUPT_REQUEST_REG_PTR interrupt_request_reg;
#define INTERRUPT_REQUEST_REGISTER_OFFSET 0x200
#define INTERRUPT_REQUEST_REGISTER_RESET 0x0



typedef union in_service_reg
{
	UINT32 vectors[8]; //16-255  32*8 = 256
	UINT32 reserved: 16; //0-15
}IN_SERVICE_REG, * IN_SERVICE_REG_PTR;

IN_SERVICE_REG_PTR in_service_reg;
#define IN_SERVICE_REGISTER_OFFSET 0x100
#define IN_SERVICE_REGISTER_RESET 0x0



typedef union trigger_mode_reg
{
	UINT32 vectors[8]; //16-255  32*8 = 256
	UINT32 reserved: 16; //0-15
}TRIGGER_MODE_REG, * TRIGGER_MODE_REG_PTR;

TRIGGER_MODE_REG_PTR trigger_mode_reg;
#define TRIGGER_MODE_REGISTER_OFFSET 0x180
#define TRIGGER_MODE_REGISTER_RESET 0x0




typedef struct eoi_reg
{
	UINT32 zero;
}EOI_REG, * EOI_REG_PTR;

EOI_REG_PTR eoi_reg;
#define EOI_REGISTER_OFFSET 0xB0
#define EOI_REGISTER_RESET 0x0




/*!
	\brief	Detects if APIC support is present on the system.

	\param	 

	\return	 0:  Success, APIC is present.
			 -1: Failure, APIC is absent.
*/
int DetectApic()
{
	INT32 present;

	/* EAX=1 will return if x2APIC support is present or not in 21st bit of output_low*/

	present = (INT32)GetFromCpuId(LOCAL_APIC_PRESENT);
	if(present) /* APIC is not present in the system */
		return -1;
	return 0;
}



/*!
	\brief	 Enables or Disables APIC functionality.

	\param	 

	\return	 void
*/
void UseApic(int enable)
{
	if(enable)
	{
		ia32_apic_base_msr->enable = 1;
	}
	else
	{
		ia32_apic_base_msr->enable = 0;
	}
}



/*!
	\brief	Provides the following support:
			1: To send an interrupt to another processor.
			2: To allow a processor to forward an interrupt, that it received but did not service, to another processor for servicing.
			3: To direct the processor to interrupt itself (perform a self interrupt).
			4: To deliver special IPIs, such as the start-up IPI (SIPI) message, to other processors.

	\param	 apic_id

	\return	 
*/
INT32 IssueInterprocessorInterrupt(UINT32 vector, UINT32 apic_id)
{
	INTERRUPT_COMMAND_REGISTER temp;
	int my_apic_id;

	temp.vector = vector;
	temp.delivery_mode = FIXED;
	temp.destination_mode = PHYSICAL;
	temp.delivery_status = IDLE;
	temp.level = ASSERT;
	temp.trigger_mode = EDGE;

	my_apic_id = (INT32)GetFromCpuId(APIC_ID);
	if( my_apic_id == apic_id)
			temp.destination_shorthand = ALL_EXCLUDING_SELF;

	/* Now copt this register contents to actual location of interrupt command register */
	memcpy( interrupt_command_register, &temp, sizeof(INTERRUPT_COMMAND_REGISTER) ); //dst, src, len
	return 0;
}


/*!
	\brief	This routine memory maps all the APIC registers from the specified starting base.

	\param	 base_address: starting address of APIC registers.

	\return	 void
*/
void InitAllApicRegisters(UINT32 base_address)
{
	interrupt_command_register = (INTERRUPT_COMMAND_REGISTER_PTR)(base_address + INTERRUPT_COMMAND_REGISTER_LOW_OFFSET);
	timer_register = (TIMER_REGISTER_PTR)(TIMER_REGISTER_OFFSET + base_address);
	lint0_reg = (LINT0_REG_PTR)(base_address + LINT0_REGISTER_OFFSET);
	lint1_reg = (LINT1_REG_PTR)(base_address + LINT1_REGISTER_OFFSET);
	error_reg = (ERROR_REG_PTR)(base_address + ERROR_REGISTER_OFFSET);
	performance_monitor_count_reg = (PERFORMANCE_MONITOR_COUNT_REG_PTR)(base_address + PERF_MON_CNT_REGISTER_OFFSET);
	thermal_sensor_reg = (THERMAL_SENSOR_REG_PTR)(base_address + THERMAL_SENSOR_REGISTER_OFFSET);
	error_status_reg = (ERROR_STATUS_REG_PTR)(base_address + ERROR_STATUS_REGISTER_OFFSET);
	local_apic_version_reg = (LOCAL_APIC_VERSION_REG_PTR)(base_address + LOCAl_APIC_VERSION_REGISTER_OFFSET);
	logical_destination_reg = (LOGICAL_DESTINATION_REG_PTR)(base_address + LOGICAL_DESTINATION_REGISTER_OFFSET);
	destination_format_reg = (DESTINATION_FORMAT_REG_PTR)(base_address + DESTINATION_FORMAT_REGISTER_OFFSET);
	arbitration_priority_reg = (ARBITRATION_PRIORITY_REG_PTR)(base_address + ARBITRATION_PRIORITY_REGISTER_OFFSET);
	task_priority_reg = (TASK_PRIORITY_REG_PTR)(base_address + TASK_PRIORITY_REGISTER_OFFSET);
	processor_priority_reg = (PROCESSOR_PRIORITY_REG_PTR)(base_address + PROCESSOR_PRIORITY_REGISTER_OFFSET);
	interrupt_request_reg = (INTERRUPT_REQUEST_REG_PTR)(base_address + INTERRUPT_REQUEST_REGISTER_OFFSET);
	in_service_reg = (IN_SERVICE_REG_PTR)(base_address + IN_SERVICE_REGISTER_OFFSET);
	trigger_mode_reg = (TRIGGER_MODE_REG_PTR)(base_address + TRIGGER_MODE_REGISTER_OFFSET);
	eoi_reg = (EOI_REG_PTR)(base_address + EOI_REGISTER_OFFSET);
	return;
}

/*!
	\brief	Sets the base address of apic to this new address.
			It then calls InitAllApicRegisters to set all APIC registers to offset from the new base address.

	\param	 addr: The new base address of apic registers.

	\return	 void
*/
void UpdateBaseApicAddress(UINT32 addr)
{
	/* backup the present contents of base register */
	IA32_APIC_BASE_MSR temp;
	temp.bsp = ia32_apic_base_msr->bsp;
	temp.enable = ia32_apic_base_msr->enable;

	/* Change the base address to new address */
	ia32_apic_base_msr = (IA32_APIC_BASE_MSR_PTR)(addr);
	temp.base = (UINT32)ia32_apic_base_msr;
	memcpy((void*)(ia32_apic_base_msr), (void*)(&temp), sizeof(IA32_APIC_BASE_MSR));

	InitAllApicRegisters(addr);
}

