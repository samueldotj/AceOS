/*!
  \file		kernel/pic/apic.c
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created:	Sat Jun 14, 2008  06:16PM
  			Last modified: Thu Aug 07, 2008  04:18PM
  \brief	Provides support for Advanced programmable interrupt controller on P4 machine.
*/

#include <ace.h>
#include <kernel/i386/cpuid.h>
#include <string.h>
#include <kernel/apic.h>
#include <kernel/ioapic.h>
#include <kernel/acpi/acpi.h>
#include <kernel/debug.h>
#include <kernel/arch.h>
#include <kernel/processor.h>
#include <kernel/mm/vm.h>
#include <kernel/i386/pmem.h>
#include <kernel/io.h>

static void BootOtherProcessors(void);
static void StartProcessor(UINT32 apic_id);
static void InitLAPICRegisters(UINT32 base_address);
static void InitLAPIC(void);
static void GetProcessorInfoFromACPI(void);

#define X2APIC_ENABLE_BIT 21
#define APIC_ENABLE_BIT 9
#define LVT_VERSION_REGISTER_OFFSET 			0x30
#define LOGICAL_DESTINATION_REGISTER_OFFSET		0xD0
#define DESTINATION_FORMAT_REGISTER_OFFSET		0xE0

IA32_APIC_BASE_MSR_PTR lapic_base_msr;

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
	\brief	Detects if APIC support is present on the processor by looking into CPUID.

	\param	 cpu_id: id of the processor which is to be queried.

	\return	 0:  Success, APIC is present.
			 -1: Failure, APIC is absent.
*/
int DetectAPIC(UINT8 cpu_id)
{
	char present;

	present = CPU_FEATURE_APIC(cpu_id);
	if(!present) /* APIC is not present on this processor */
		return -1;
	return 0;
}

/*!
	\brief	 Enables or Disables APIC functionality.

	\param	 

	\return	 void
*/
void UseAPIC(int enable)
{
	if(enable)
	{
		lapic_base_msr->enable = 1;
		_outp(0x70, 0x22);
		_outp(0x01, 0x23);
	}
	else
	{
		lapic_base_msr->enable = 0;
	}
}

/*!
	\brief	Initialize APIC by getting information from ACPI. This is called from main.c.
			This initializes LAPIC and IOAPIC by calling their respective functions.

	\param	 void

	\return	 void
*/
void InitAPIC(void)
{
	GetProcessorInfoFromACPI();
	//Init lapic and ioapic registers.
	InitLAPIC();
	InitIOAPIC();
	UseAPIC(1);
	kprintf("APIC is in use now\n");
	return;
}

/*!
	\brief	The act of writing anything to this register will cause an EOI to be issued.

	\param	 int_no: Interrupt number

	\return	 void
*/
void SendEndOfInterrupt(int int_no)
{
	eoi_reg->zero = 0;
}


/*!
	\brief	 Initialize LAPIC registers and LAPIC base address.

	\param	 void

	\return	 void
*/
static void InitLAPIC(void)
{
	UINT32 va, kernel_stack_pages=2;

	if ( AllocateVirtualMemory(&kernel_map, &va, 0, PAGE_SIZE * kernel_stack_pages, 0, 0) != ERROR_SUCCESS )
		panic("VA not available for starting secondary CPU\n");
	lapic_base_msr = (IA32_APIC_BASE_MSR_PTR)va;

	if ( CreatePhysicalMapping(kernel_map.physical_map, (UINT32)lapic_base_msr, va, 0) != ERROR_SUCCESS )
		panic("VA to PA mapping failed\n");

	InitLAPICRegisters(LAPIC_BASE_MSR_START);
	return;
}

/*!
	\brief	Sets the base address of LAPIC to this new address.

	\param	 addr: The new physical base address of LAPIC base register.

	\return	 void
*/
void RelocateBaseLAPICAddress(UINT32 addr)
{
	/* backup the present contents of base register */
	IA32_APIC_BASE_MSR temp;
	temp.bsp = lapic_base_msr->bsp;
	temp.enable = lapic_base_msr->enable;

	if ( CreatePhysicalMapping(kernel_map.physical_map, (UINT32)lapic_base_msr, addr, 0) != ERROR_SUCCESS )
		panic("VA to PA mapping failed\n");

	/* Change the base address to new address */
	lapic_base_msr = (IA32_APIC_BASE_MSR_PTR)(addr);
	temp.base_low = lapic_base_msr->base_low;
	temp.base_high = lapic_base_msr->base_high;

	memcpy((void*)(lapic_base_msr), (void*)(&temp), sizeof(IA32_APIC_BASE_MSR));
}


/*!
	\brief	 Initialise SMP environment

	\param	 void

	\return	 void

	\Assumption: We assume that InitAPIC() is already called and processor structures are updated.
*/
void InitSmp(void)
{
    BootOtherProcessors();
}



/*!
	\brief	 Boot all application processors by calling StartProcessor for each of the processors on the system.

	\param	 void

	\return	 void
*/
static void BootOtherProcessors(void)
{
	UINT32 apic_id, processor_count;
    //Send SIPI to all cpu's
    for(processor_count=0; processor_count < count_running_processors ; processor_count++)
	{
		//if (processor[processor_count].state == OFFLINE)
		apic_id = processor[processor_count].apic_id;
	    StartProcessor(apic_id);
	}
}


/*!
	\brief	Start a application processor by issuing IPI messages in the order: INIT, SIPI, SIPI.

	\param	apic_id: apic id of the processor which is to be started.

	\return	 void
*/
static void StartProcessor(UINT32 apic_id)
{
	int temp_count_processors = count_running_processors;
	int temp_loop;

	/* Get the 32 bit physical address which contains the code to execute on ap's.  We need only first 8 bits(LSB) of the physical address. */
    UINT8 vector = (CreatePageForSecondaryCPUStart() & 0xff); 

	/* BSP should initialize the BIOS shutdown code to 0AH and vector to startup code. */
	//TBD
	IssueInterprocessorInterrupt(vector, apic_id, ICR_DELIVERY_MODE_INIT, ICR_DESTINATION_SHORTHAND_NO_SHORTHAND, 0);

	//delay(10); //I want to sleep for 10m sec
	for(temp_loop=0; temp_loop < 1000000; temp_loop++); //just an approximate.

	IssueInterprocessorInterrupt(vector, apic_id, ICR_DELIVERY_MODE_SIPI, ICR_DESTINATION_SHORTHAND_NO_SHORTHAND, 0);
	
	//delay(200Micr sec);
	for(temp_loop=0; temp_loop < 20000; temp_loop++); //just an approximate
	
	IssueInterprocessorInterrupt(vector, apic_id, ICR_DELIVERY_MODE_SIPI, ICR_DESTINATION_SHORTHAND_NO_SHORTHAND, 0);
	
	//delay(200Micr sec);
	for(temp_loop=0; temp_loop < 20000; temp_loop++); //Just an approximate
	
	//Now check if AP has started running?
	if ( temp_count_processors != (count_running_processors + 1) )
	{
		kprintf("Something wrong in booting ap %d!\n", apic_id);
		//mark the processor as absent.
	}
	return;
}


/*!
	\brief	This routine memory maps all the LAPIC registers from the specified starting base.

	\param	 base_address: starting address of LAPIC registers.

	\return	 void
*/
static void InitLAPICRegisters(UINT32 base_address)
{
	interrupt_command_register		=	(INTERRUPT_COMMAND_REGISTER_PTR)(base_address + INTERRUPT_COMMAND_REGISTER_LOW_OFFSET);
	timer_register 					=	(TIMER_REGISTER_PTR)(TIMER_REGISTER_OFFSET + base_address);
	lint0_reg						=	(LINT0_REG_PTR)(base_address + LINT0_REGISTER_OFFSET);
	lint1_reg						=	(LINT1_REG_PTR)(base_address + LINT1_REGISTER_OFFSET);
	error_reg						=	(ERROR_REG_PTR)(base_address + ERROR_REGISTER_OFFSET);
	performance_monitor_count_reg	=	(PERFORMANCE_MONITOR_COUNT_REG_PTR)(base_address + PERF_MON_CNT_REGISTER_OFFSET);
	thermal_sensor_reg				=	(THERMAL_SENSOR_REG_PTR)(base_address + THERMAL_SENSOR_REGISTER_OFFSET);
	error_status_reg				=	(ERROR_STATUS_REG_PTR)(base_address + ERROR_STATUS_REGISTER_OFFSET);
	local_apic_version_reg			=	(LOCAL_APIC_VERSION_REG_PTR)(base_address + LOCAl_APIC_VERSION_REGISTER_OFFSET);
	logical_destination_reg			=	(LOGICAL_DESTINATION_REG_PTR)(base_address + LOGICAL_DESTINATION_REGISTER_OFFSET);
	destination_format_reg			=	(DESTINATION_FORMAT_REG_PTR)(base_address + DESTINATION_FORMAT_REGISTER_OFFSET);
	arbitration_priority_reg		=	(ARBITRATION_PRIORITY_REG_PTR)(base_address + ARBITRATION_PRIORITY_REGISTER_OFFSET);
	task_priority_reg				=	(TASK_PRIORITY_REG_PTR)(base_address + TASK_PRIORITY_REGISTER_OFFSET);
	processor_priority_reg			=	(PROCESSOR_PRIORITY_REG_PTR)(base_address + PROCESSOR_PRIORITY_REGISTER_OFFSET);
	interrupt_request_reg			=	(INTERRUPT_REQUEST_REG_PTR)(base_address + INTERRUPT_REQUEST_REGISTER_OFFSET);
	in_service_reg					=	(IN_SERVICE_REG_PTR)(base_address + IN_SERVICE_REGISTER_OFFSET);
	trigger_mode_reg				=	(TRIGGER_MODE_REG_PTR)(base_address + TRIGGER_MODE_REGISTER_OFFSET);
	eoi_reg							=	(EOI_REG_PTR)(base_address + EOI_REGISTER_OFFSET);
	return;
}


/*!
	\brief	 Get apic information by calling ACPI.

	\param	 void

	\return	 void
*/
static void GetProcessorInfoFromACPI(void)
{
	ACPI_TABLE_MADT *madt_ptr;
	if ( AcpiGetTable ("APIC", 1, (ACPI_TABLE_HEADER**)(&madt_ptr)) != AE_OK )
		kprintf("AcpiGetTable() failed\n");
	else
	{
		kprintf("LAPIC Address %p [%s]\n", madt_ptr->Address, madt_ptr->Flags&1?"APIC and Dual 8259 Support":"Only APIC" );
		ACPI_SUBTABLE_HEADER *sub_header, *table_end;
		sub_header = (ACPI_SUBTABLE_HEADER *) ( ((UINT32)madt_ptr) + sizeof(ACPI_TABLE_MADT) );
		table_end = (ACPI_SUBTABLE_HEADER *) ( ((UINT32)madt_ptr) + madt_ptr->Header.Length );

		count_running_processors = 0;
		while ( sub_header < table_end )
		{
			if ( sub_header->Type == ACPI_MADT_TYPE_LOCAL_APIC )
			{
				ACPI_MADT_LOCAL_APIC *p = ( ACPI_MADT_LOCAL_APIC * ) sub_header;
				kprintf("Processor ID %d LAPIC Id = %d [%s]\n", p->ProcessorId, p->Id, ( (p->LapicFlags & 1) ? "Online" : "Offline") );
				processor[count_running_processors].apic_id = p->Id;
				processor[count_running_processors].state = ( (p->LapicFlags & 1) ? (PROCESSOR_STATE_ONLINE) : (PROCESSOR_STATE_OFFLINE) );
				kprintf("processor loaded..\n");
				count_running_processors++;
			}
			else if ( sub_header->Type == ACPI_MADT_TYPE_IO_APIC )
			{
				ACPI_MADT_IO_APIC *p = ( ACPI_MADT_IO_APIC * ) sub_header;
				kprintf("IOAPIC ID %d IOAPIC Physical Address = %p GlobalIRQBase %d\n", p->Id, p->Address, p->GlobalIrqBase);
				ioapic[count_ioapic].ioapic_id = p->Id;
				ioapic[count_ioapic].physical_address = p->Address;
				ioapic[count_ioapic].starting_vector = p->GlobalIrqBase;
				count_ioapic++;
			}
			else
				kprintf("Type = %d\n", sub_header->Type);

			sub_header = (ACPI_SUBTABLE_HEADER *) ( ((UINT32)sub_header) + sub_header->Length );
		}
		kprintf("while done\n");
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
	cmd.destination_mode = ICR_DESTINATION_MODE_PHYSICAL;
	cmd.level = ICR_LEVEL_ASSERT;
	cmd.destination_field = apic_id;

	switch(delivery_mode)
	{
		case ICR_DELIVERY_MODE_FIXED:			break;
		case ICR_DELIVERY_MODE_LOWEST_PRIORITY:	break;
		case ICR_DELIVERY_MODE_SMI:				cmd.vector = 0; /*This is for future compatibility as described in specs. */
												break;
		case ICR_DELIVERY_MODE_NMI:				break;
		case ICR_DELIVERY_MODE_INIT:
												if(init_de_assert) {
													cmd.level = ICR_LEVEL_DE_ASSERT;
												}		
												cmd.trigger_mode = ICR_TRIGGER_MODE_LEVEL;
												cmd.vector = 0; /*This is for future compatibility as described in specs. */
												break;
		case ICR_DELIVERY_MODE_SIPI:			cmd.trigger_mode = ICR_TRIGGER_MODE_EDGE;
												break;
		case ICR_DELIVERY_MODE_ExtINT: 			cmd.trigger_mode = ICR_TRIGGER_MODE_LEVEL; break;
		default:								break;
	}
	
	switch(destination_shorthand)
	{
    	case	ICR_DESTINATION_SHORTHAND_NO_SHORTHAND:			break;
		case	ICR_DESTINATION_SHORTHAND_SELF:					break;
		case	ICR_DESTINATION_SHORTHAND_ALL_INCLUDING_SELF:	cmd.destination_field = 0XFF;
																break;
		case	ICR_DESTINATION_SHORTHAND_ALL_EXCLUDING_SELF:	cmd.destination_field = 0XFF;
																break;
	}

	/* Now copy these register contents to actual location of interrupt command register */
	memcpy( interrupt_command_register, &cmd, sizeof(INTERRUPT_COMMAND_REGISTER) ); //dst, src, len
	/* This act of writing into ICR will make APIC to generate an interrupt */
	return 0;
}
