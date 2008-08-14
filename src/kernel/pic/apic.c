/*!
  \file		kernel/pic/apic.c
  \brief	Provides support for Advanced programmable interrupt controller on P4 machine.
*/

#include <ace.h>
#include <string.h>
#include <kernel/io.h>
#include <kernel/debug.h>
#include <kernel/processor.h>
#include <kernel/arch.h>
#include <kernel/apic.h>
#include <kernel/ioapic.h>
#include <kernel/mm/vm.h>
#include <kernel/i386/cpuid.h>
#include <kernel/i386/pmem.h>

static void BootOtherProcessors(void);
static void StartProcessor(UINT32 apic_id);
static void InitLAPIC(void);

#define X2APIC_ENABLE_BIT										21
#define APIC_ENABLE_BIT 										9

IA32_APIC_BASE_MSR_PTR lapic_base_msr;

#define LVT_VERSION_REGISTER_OFFSET 							0x30
#define LOGICAL_DESTINATION_REGISTER_OFFSET						0xD0
#define DESTINATION_FORMAT_REGISTER_OFFSET						0xE0

#define LOCAl_APIC_VERSION_REGISTER_OFFSET 						0x30
#define LOCAl_APIC_VERSION_REGISTER_ADDRESS(lapic_base_address)	((LOCAL_APIC_VERSION_REG_PTR) ((UINT32)(lapic_base_address) + LOCAl_APIC_VERSION_REGISTER_OFFSET) )
#define COUNT_ENTRIES_LVT(lapic_base_address)					(LOCAl_APIC_VERSION_REGISTER_ADDRESS(lapic_base_address)->max_lvt_entry + 1)

#define TASK_PRIORITY_REGISTER_OFFSET							0x80
#define TASK_PRIORITY_REGISTER_RESET							0x0
#define TASK_PRIORITY_REGISTER_ADDRESS(lapic_base_address)		((TASK_PRIORITY_REG_PTR) ((UINT32)(lapic_base_address) + TASK_PRIORITY_REGISTER_OFFSET) )

#define ARBITRATION_PRIORITY_REGISTER_OFFSET					0x90
#define ARBITRATION_PRIORITY_REGISTER_RESET 					0x0
#define ARBITRATION_PRIORITY_REGISTER_ADDRESS(lapic_base_address) ((ARBITRATION_PRIORITY_REG_PTR) ((UINT32)(lapic_base_address) + ARBITRATION_PRIORITY_REGISTER_OFFSET) )

#define PROCESSOR_PRIORITY_REGISTER_OFFSET						0xA0
#define PROCESSOR_PRIORITY_REGISTER_RESET 						0x0
#define PROCESSOR_PRIORITY_REGISTER_ADDRESS(lapic_base_address)	((PROCESSOR_PRIORITY_REG_PTR) ((UINT32)(lapic_base_address) + PROCESSOR_PRIORITY_REGISTER_OFFSET) )

#define EOI_REGISTER_OFFSET										0xB0
#define EOI_REGISTER_RESET										0x0
#define EOI_REGISTER_ADDRESS(lapic_base_address)				((EOI_REG_PTR) ((UINT32)(lapic_base_address) + EOI_REGISTER_OFFSET) )

#define LOGICAL_DESTINATION_REGISTER_OFFSET 					0xD0
#define LOGICAL_DESTINATION_REGISTER_RESET 						0x0
#define LOGICAL_DESTINATION_REGISTER_ADDRESS(lapic_base_address) ((LOGICAL_DESTINATION_REG_PTR) ((UINT32)(lapic_base_address) + LOGICAL_DESTINATION_REGISTER_OFFSET) )

#define DESTINATION_FORMAT_REGISTER_OFFSET 						0xE0
#define DESTINATION_FORMAT_REGISTER_RESET 						0xFFFFFFFF
#define DESTINATION_FORMAT_REGISTER_ADDRESS(lapic_base_address)	((DESTINATION_FORMAT_REG_PTR) ((UINT32)(lapic_base_address) + DESTINATION_FORMAT_REGISTER_OFFSET) )

#define SPURIOUS_INTERRUPT_VECTOR_REGISTER_OFFSET				0xF0
#define SPURIOUS_INTERRUPT_VECTOR_REGISTER_ADDRESS(lapic_base_address)	\
																((SPURIOUS_INTERRUPT_REG_PTR) ((UINT32)(lapic_base_address) + SPURIOUS_INTERRUPT_VECTOR_REGISTER_OFFSET) )

#define IN_SERVICE_REGISTER_OFFSET								0x100
#define IN_SERVICE_REGISTER_RESET								0x0
#define IN_SERVICE_REGISTER_ADDRESS(lapic_base_address)			((IN_SERVICE_REG_PTR) ((UINT32)(lapic_base_address) + IN_SERVICE_REGISTER_OFFSET) )

#define TRIGGER_MODE_REGISTER_OFFSET							0x180
#define TRIGGER_MODE_REGISTER_RESET								0x0
#define TRIGGER_MODE_REGISTER_ADDRESS(lapic_base_address)		(TRIGGER_MODE_REG_PTR) ((UINT32)(lapic_base_address) + TRIGGER_MODE_REGISTER_OFFSET) )

#define INTERRUPT_REQUEST_REGISTER_OFFSET						0x200
#define INTERRUPT_REQUEST_REGISTER_RESET						0x0
#define INTERRUPT_REQUEST_REGISTER_ADDRESS(lapic_base_address)	((INTERRUPT_REQUEST_REG_PTR) ((UINT32)(lapic_base_address) + INTERRUPT_REQUEST_REGISTER_OFFSET) )

#define ERROR_STATUS_REGISTER_OFFSET							0x280
#define ERROR_STATUS_REGISTER_RESET								0x0
#define ERROR_STATUS_REGISTER_ADDRESS(lapic_base_address)		((ERROR_STATUS_REG_PTR) ((UINT32)(lapic_base_address) + ERROR_STATUS_REGISTER_OFFSET) )

#define INTERRUPT_COMMAND_REGISTER_LOW_OFFSET					0x300 /*(0-31 bits)*/
#define INTERRUPT_COMMAND_REGISTER_HIGH_OFFSET					0x310 /*(32-63 bits)*/
#define INTERRUPT_COMMAND_REGISTER_ADDRESS(lapic_base_address) 	((INTERRUPT_COMMAND_REGISTER_PTR)( (UINT32)(lapic_base_address) + INTERRUPT_COMMAND_REGISTER_LOW_OFFSET ))

#define TIMER_REGISTER_OFFSET 									0x320
#define TIMER_REGISTER_RESET 									0x00010000
#define TIMER_REGISTER_ADDRESS(lapic_base_address)				((TIMER_REGISTER_PTR) ((UINT32)(lapic_base_address) + TIMER_REGISTER_OFFSET) )

#define THERMAL_SENSOR_REGISTER_OFFSET							0x330
#define THERMAL_SENSOR_REGISTER_RESET							0x00010000
#define THERMAL_SENSOR_REG_ADDRESS(lapic_base_address)			((THERMAL_SENSOR_REG_PTR) ((UINT32)(lapic_base_address) + THERMAL_SENSOR_REGISTER_OFFSET) )

#define PERF_MON_CNT_REGISTER_OFFSET 							0x340
#define PERF_MON_CNT_REGISTER_RESET 							0x00010000
#define PERF_MON_CNT_REGISTER_ADDRESS(lapic_base_address)		((PERFORMANCE_MONITOR_COUNT_REG_PTR) ((UINT32)(lapic_base_address) + PERF_MON_CNT_REGISTER_OFFSET) )

#define LINT0_REGISTER_OFFSET 									0x350
#define LINT0_REGISTER_RESET									0x00010000
#define LINT0_REGISTER_ADDRESS(lapic_base_address)				((LINT0_REG_PTR) ((UINT32)(lapic_base_address) + LINT0_REGISTER_OFFSET) )

#define LINT1_REGISTER_OFFSET 									0x360
#define LINT1_REGISTER_RESET 									0x00010000
#define LINT1_REGISTER_ADDRESS(lapic_base_address)				((LINT1_REG_PTR) ((UINT32)(lapic_base_address) + LINT1_REGISTER_OFFSET) )

#define ERROR_REGISTER_OFFSET 									0x370
#define ERROR_REGISTER_RESET 									0x00010000
#define ERROR_REGISTER_ADDRESS(lapic_base_address)				((ERROR_REG_PTR) ((UINT32)(lapic_base_address) + ERROR_REGISTER_OFFSET) )

#define ACPI_MEMORY_MAP_SIZE									0x400


#define SPURIOUS_VECTOR_NUMBER									240
#define LINT0_VECTOR_NUMBER										241
#define LINT1_VECTOR_NUMBER										242
#define ERROR_VECTOR_NUMBER										243
#define PERF_MON_VECTOR_NUMBER									244
#define THERMAL_SENSOR_VECTOR_NUMBER							245



/*!
 *	\brief	 Initializes LAPICT and IOAPIC. This function is called by the processor intialization code. kernel/i386/SetupAPIC()
*/
inline void InitAPIC(void)
{
	InitLAPIC();
	InitIOAPIC();
}

/*! Initialize LAPIC to receive interrupts.
 *		1) Enable APIC
 *		2) Mask interrupts for LINT0, LINT1, Error, Performance Monitors and Thermal sensors. These registers should be enabled later.
 *		3) Set Task Priority to 0, so that all interrupts will be received
*/
static void InitLAPIC()
{
	volatile SPURIOUS_INTERRUPT_REG_PTR _sir;
	volatile LINT0_REG_PTR _lint0;
	volatile LINT1_REG_PTR _lint1;
	volatile TASK_PRIORITY_REG_PTR _tpr;
	volatile ERROR_REG_PTR _er;
	volatile PERFORMANCE_MONITOR_COUNT_REG_PTR _pr;
	
	/*! Setup the base register of APIC */
	lapic_base_msr = (IA32_APIC_BASE_MSR_PTR)MapPhysicalMemory(&kernel_map, LAPIC_BASE_MSR_START, sizeof(IA32_APIC_BASE_MSR)); 
	/*! \todo size is not this.. it should be sum total of the sizes of all register structures. */

	/*! enable APIC */
	_sir =	SPURIOUS_INTERRUPT_VECTOR_REGISTER_ADDRESS(lapic_base_msr);
	_sir->apic_enable = 1;
	_sir->spurious_vector = SPURIOUS_VECTOR_NUMBER;
	
	/*! Mask LINT0 and LINT1 interrupts */
	_lint0 = LINT0_REGISTER_ADDRESS(lapic_base_msr);
	_lint0->mask = 0;
	_lint0->vector = LINT0_VECTOR_NUMBER;
	
	_lint1 = LINT1_REGISTER_ADDRESS(lapic_base_msr);
	_lint1->mask = 0;
	_lint1->vector = LINT1_VECTOR_NUMBER;
	
	/*! mask error register */
	_er = ERROR_REGISTER_ADDRESS(lapic_base_msr);
	_er->mask = 0;
	_er->vector = ERROR_VECTOR_NUMBER;
	
	/*! mask performance monitoring registers */
	_pr = PERF_MON_CNT_REGISTER_ADDRESS(lapic_base_msr);
	_pr->mask = 0;
	_pr->vector = PERF_MON_VECTOR_NUMBER;
	
	/*! set task priority to 0 to receive all interrupts */
	_tpr = TASK_PRIORITY_REGISTER_ADDRESS(lapic_base_msr);
	_tpr->task_priority=0;
}

/*!
 *	\brief			The act of writing anything to this register will cause an EOI to be issued.
 *	\param	int_no	Interrupt number
*/
void SendEndOfInterrupt(int int_no)
{
	EOI_REG_PTR er;
	er = EOI_REGISTER_ADDRESS(lapic_base_msr);
	er->zero = 0;
}


/*!
 *	\brief	 Initialise SMP environment
 *	\note	Assumption: We assume that InitAPIC() is already called and processor structures are updated.
*/
void InitSmp(void)
{
    BootOtherProcessors();
}


/*!
 *	\brief	 Boot all application processors by calling StartProcessor for each of the processors on the system.
*/
static void BootOtherProcessors(void)
{
	UINT32 apic_id, processor_count;
    /*! Send SIPI to all cpu's */
    for(processor_count=0; processor_count < count_running_processors ; processor_count++)
	{
		/*! if (processor[processor_count].state == OFFLINE) */
		apic_id = processor[processor_count].apic_id;
	    StartProcessor(apic_id);
	}
}


/*!
 *	\brief			Start a application processor by issuing IPI messages in the order: INIT, SIPI, SIPI.
 *	\param	apic_id	apic id of the processor which is to be started.
*/
static void StartProcessor(UINT32 apic_id)
{
	int temp_count_processors = count_running_processors;
	int temp_loop;

	/*! Get the 32 bit physical address which contains the code to execute on ap's.  We need only first 8 bits(LSB) of the physical address. */
    UINT8 vector = (CreatePageForSecondaryCPUStart() & 0xff); 

	IssueInterprocessorInterrupt(vector, apic_id, ICR_DELIVERY_MODE_INIT, ICR_DESTINATION_SHORTHAND_NO_SHORTHAND);

	/*! delay(10); //I want to sleep for 10m sec */
	for(temp_loop=0; temp_loop < 1000000; temp_loop++); /*! just an approximate. */

	IssueInterprocessorInterrupt(vector, apic_id, ICR_DELIVERY_MODE_SIPI, ICR_DESTINATION_SHORTHAND_NO_SHORTHAND);
	
	/*! delay(200Micr sec); */
	for(temp_loop=0; temp_loop < 20000; temp_loop++); //just an approximate
	
	IssueInterprocessorInterrupt(vector, apic_id, ICR_DELIVERY_MODE_SIPI, ICR_DESTINATION_SHORTHAND_NO_SHORTHAND);
	
	/*! delay(200Micr sec); */
	for(temp_loop=0; temp_loop < 20000; temp_loop++); /*!Just an approximate */
	
	/*! Now check if AP has started running? */
	if ( temp_count_processors != (count_running_processors + 1) )
	{
		kprintf("Something wrong in booting ap %d!\n", apic_id);
		/*! mark the processor as absent. */
	}
	return;
}


/*!
 *	\brief	Provides the following support:
 *			1: To send an interrupt to another processor.
 *			2: To allow a processor to forward an interrupt, that it received but did not service, to another processor for servicing.
 *			3: To direct the processor to interrupt itself (perform a self interrupt).
 *			4: To deliver special IPIs, such as the start-up IPI (SIPI) message, to other processors.
 *
 *	\param	vector					Vector number of the interrupt being sent. 
 *	\param	apic_id					apic id of the processor to which an interrupt is to be sent.
 *	\param	delivery_mode			Specifies the type of IPI to be sent.
 *	\param	destination_shorthand	A shorthand nottation to send the message.
*/
void IssueInterprocessorInterrupt(UINT32 vector, UINT32 apic_id, enum ICR_DELIVERY_MODE delivery_mode,
				enum ICR_DESTINATION_SHORTHAND destination_shorthand)
{
	INTERRUPT_COMMAND_REGISTER cmd;
	INTERRUPT_COMMAND_REGISTER_PTR _icr_reg;
	
	cmd.vector = vector;
	cmd.delivery_mode = delivery_mode;
	cmd.destination_mode = ICR_DESTINATION_MODE_PHYSICAL;
	cmd.level = ICR_LEVEL_ASSERT;
	cmd.destination_field = apic_id;

	switch(delivery_mode)
	{
		case ICR_DELIVERY_MODE_FIXED:			break;
		case ICR_DELIVERY_MODE_LOWEST_PRIORITY:	break;
		case ICR_DELIVERY_MODE_SMI:				cmd.vector = 0; /*! This is for future compatibility as described in specs. */
												break;
		case ICR_DELIVERY_MODE_NMI:				break;
		case ICR_DELIVERY_MODE_INIT:
												cmd.trigger_mode = ICR_TRIGGER_MODE_LEVEL;
												cmd.vector = 0; /*! This is for future compatibility as described in specs. */
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

	_icr_reg = INTERRUPT_COMMAND_REGISTER_ADDRESS(lapic_base_msr);
	/*! Now copy these register contents to actual location of interrupt command register */
	memcpy( _icr_reg, &cmd, sizeof(INTERRUPT_COMMAND_REGISTER) ); //dst, src, len
	/*! This act of writing into ICR will make APIC to generate an interrupt */
}
