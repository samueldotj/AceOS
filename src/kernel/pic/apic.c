/*!
  \file		kernel/pic/apic.c
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created:	Sat Jun 14, 2008  06:16PM
  			Last modified: Tue Jul 29, 2008  02:46PM
  \brief	Provides support for Advanced programmable interrupt controller on P4 machine.
*/

#include <ace.h>
#include <kernel/i386/cpuid.h>
#include <string.h>
#include <kernel/apic.h>

#define X2APIC_ENABLE_BIT 21
#define APIC_ENABLE_BIT 9
#define LVT_VERSION_REGISTER_OFFSET 			0x30
#define LOGICAL_DESTINATION_REGISTER_OFFSET		0xD0
#define DESTINATION_FORMAT_REGISTER_OFFSET		0xE0

IA32_APIC_BASE_MSR_PTR ia32_apic_base_msr;
IA32_APIC_BASE_MSR_PTR ia32_ioapic_base_msr;
#define IA32_APIC_BASE_MSR_START 0xfee00000
#define IA32_IOAPIC_BASE_MSR_START 0xfec00000

#define AM_I_BOOTSTRAP_PROCESSOR ia32_apic_base_msr.bsp

INTERRUPT_COMMAND_REGISTER_PTR interrupt_command_register;
#define INTERRUPT_COMMAND_REGISTER_LOW_OFFSET	0x300 /*(0-31 bits)*/
#define INTERRUPT_COMMAND_REGISTER_HIGH_OFFSET	0x310 /*(32-63 bits)*/

TIMER_REGISTER_PTR timer_register;
#define TIMER_REGISTER_OFFSET 0x320
#define TIMER_REGISTER_RESET 0x00010000

LINT0_REG_PTR lint0_reg;
#define LINT0_REGISTER_OFFSET 0X350
#define LINT0_REGISTER_RESET 0x00010000


LINT1_REG_PTR lint1_reg;
#define LINT1_REGISTER_OFFSET 0X360
#define LINT0_REGISTER_RESET 0x00010000


ERROR_REG_PTR error_reg;
#define ERROR_REGISTER_OFFSET 0X370
#define ERROR_REGISTER_RESET 0x00010000

PERFORMANCE_MONITOR_COUNT_REG_PTR performance_monitor_count_reg;
#define PERF_MON_CNT_REGISTER_OFFSET 0X340
#define PERF_MON_CNT_REGISTER_RESET 0x00010000 


THERMAL_SENSOR_REG_PTR thermal_sensor_reg;
#define THERMAL_SENSOR_REGISTER_OFFSET 0x330
#define THERMAL_SENSOR_REGISTER_RESET 0x00010000

ERROR_STATUS_REG_PTR error_status_reg;
#define ERROR_STATUS_REGISTER_OFFSET 0x280
#define ERROR_STATUS_REGISTER_RESET 0x0

LOCAL_APIC_VERSION_REG_PTR local_apic_version_reg;
#define LOCAl_APIC_VERSION_REGISTER_OFFSET 0x30
#define COUNT_ENTRIES_LVT (local_apic_version_reg.max_lvt_entry + 1)

LOGICAL_DESTINATION_REG_PTR logical_destination_reg;
#define LOGICAL_DESTINATION_REGISTER_OFFSET 0xD0
#define LOGICAL_DESTINATION_REGISTER_RESET 0x0

DESTINATION_FORMAT_REG_PTR destination_format_reg;
#define DESTINATION_FORMAT_REGISTER_OFFSET 0xE0
#define DESTINATION_FORMAT_REGISTER_RESET 0xFFFFFFFF

ARBITRATION_PRIORITY_REG_PTR arbitration_priority_reg;
#define ARBITRATION_PRIORITY_REGISTER_OFFSET 0x90
#define ARBITRATION_PRIORITY_REGISTER_RESET 0x0

TASK_PRIORITY_REG_PTR task_priority_reg;
#define TASK_PRIORITY_REGISTER_OFFSET 0x80
#define TASK_PRIORITY_REGISTER_RESET 0x0

PROCESSOR_PRIORITY_REG_PTR processor_priority_reg;
#define PROCESSOR_PRIORITY_REGISTER_OFFSET 0xA0
#define PROCESSOR_PRIORITY_REGISTER_RESET 0x0

INTERRUPT_REQUEST_REG_PTR interrupt_request_reg;
#define INTERRUPT_REQUEST_REGISTER_OFFSET 0x200
#define INTERRUPT_REQUEST_REGISTER_RESET 0x0

IN_SERVICE_REG_PTR in_service_reg;
#define IN_SERVICE_REGISTER_OFFSET 0x100
#define IN_SERVICE_REGISTER_RESET 0x0

TRIGGER_MODE_REG_PTR trigger_mode_reg;
#define TRIGGER_MODE_REGISTER_OFFSET 0x180
#define TRIGGER_MODE_REGISTER_RESET 0x0

EOI_REG_PTR eoi_reg;
#define EOI_REGISTER_OFFSET 0xB0
#define EOI_REGISTER_RESET 0x0

/*!
	\brief	Detects if APIC support is present on the system by looking into CPUID.

	\param	 

	\return	 0:  Success, APIC is present.
			 -1: Failure, APIC is absent.
*/
int DetectApic()
{
	char present;

	present = (char)(EXTRACT_FROM_CPUID_LAPIC);
	if(!present) /* APIC is not present on this processor */
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

	\param	vector: Vector number of the interrupt being sent. 
			apic_id: apic id of the processor to which an interrupt is to be sent.
			delivery_mode: Specifies the type of IPI to be sent.
	\return	 
*/
INT16 IssueInterprocessorInterrupt(UINT32 vector, UINT32 apic_id, enum ICR_DELIVERY_MODE delivery_mode,
				enum ICR_DESTINATION_SHORTHAND destination_shorthand, BYTE init_de_assert)
{
	INTERRUPT_COMMAND_REGISTER cmd;
	
	cmd.vector = vector;
	cmd.delivery_mode = delivery_mode;
	cmd.destination_mode = PHYSICAL;
	cmd.level = ASSERT;
	cmd.destination_field = apic_id;

	switch(delivery_mode)
	{
		case FIXED: 			break;
		case LOWEST_PRIORITY: 	break;
		case SMI:				cmd.vector = 0; /*This is for future compatibility as described in specs. */
								break;
		case NMI:				break;
		case INIT:
								if(init_de_assert) {
									cmd.level = DE_ASSERT;
									cmd.trigger_mode = LEVEL;
								}		
								cmd.vector = 0; /*This is for future compatibility as described in specs. */
								break;
		case SIPI: 				break;
		case ExtINT: 			cmd.trigger_mode = LEVEL; break;
		default: 				break;
	}
	
	switch(destination_shorthand)
	{
    	case	NO_SHORTHAND:		break;
		case	SELF:				break;
		case	ALL_INCLUDING_SELF:	cmd.destination_field = 0XFF;
									break;
		case	ALL_EXCLUDING_SELF:	cmd.destination_field = 0XFF;
									break;
	}

	/* Now copy these register contents to actual location of interrupt command register */
	memcpy( interrupt_command_register, &cmd, sizeof(INTERRUPT_COMMAND_REGISTER) ); //dst, src, len
	/* This act of writing into ICR will make APIC to generate an interrupt */
	return 0;
}

/*!
	\brief	This routine memory maps all the APIC registers from the specified starting base.

	\param	 base_address: starting address of APIC registers.

	\return	 void
*/
static void InitAllApicRegisters(UINT32 base_address)
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
void RelocateBaseApicAddress(UINT32 addr)
{
	/* backup the present contents of base register */
	IA32_APIC_BASE_MSR temp;
	temp.bsp = ia32_apic_base_msr->bsp;
	temp.enable = ia32_apic_base_msr->enable;

	/* Change the base address to new address */
	ia32_apic_base_msr = (IA32_APIC_BASE_MSR_PTR)(addr);
	temp.base_low = ia32_apic_base_msr->base_low;
	temp.base_high = ia32_apic_base_msr->base_high;

	memcpy((void*)(ia32_apic_base_msr), (void*)(&temp), sizeof(IA32_APIC_BASE_MSR));

	InitAllApicRegisters(addr);
}
