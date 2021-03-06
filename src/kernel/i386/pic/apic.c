/*!
  \file		kernel/i386/pic/apic.c
  \brief	Provides support for Advanced programmable interrupt controller on P4 machine.
*/

#include <ace.h>
#include <string.h>
#include <kernel/io.h>
#include <kernel/arch.h>
#include <kernel/i386/apic.h>
#include <kernel/debug.h>
#include <kernel/i386/ioapic.h>
#include <kernel/processor.h>
#include <kernel/pit.h>
#include <kernel/mm/vm.h>
#include <kernel/acpi/acpi.h>

IA32_APIC_BASE_MSR_PTR lapic_base_address = NULL;

#define LOCAl_APIC_VERSION_REGISTER_OFFSET 						0x30
#define LOCAl_APIC_VERSION_REGISTER_ADDRESS(lapic_base_address)	((LOCAL_APIC_VERSION_REG_PTR) ((UINT32)(lapic_base_address) + LOCAl_APIC_VERSION_REGISTER_OFFSET) )

#define TASK_PRIORITY_REGISTER_OFFSET							0x80
#define TASK_PRIORITY_REGISTER_ADDRESS(lapic_base_address)		((TASK_PRIORITY_REG_PTR) ((UINT32)(lapic_base_address) + TASK_PRIORITY_REGISTER_OFFSET) )

#define ARBITRATION_PRIORITY_REGISTER_OFFSET					0x90
#define ARBITRATION_PRIORITY_REGISTER_ADDRESS(lapic_base_address) ((ARBITRATION_PRIORITY_REG_PTR) ((UINT32)(lapic_base_address) + ARBITRATION_PRIORITY_REGISTER_OFFSET) )

#define PROCESSOR_PRIORITY_REGISTER_OFFSET						0xA0
#define PROCESSOR_PRIORITY_REGISTER_ADDRESS(lapic_base_address)	((PROCESSOR_PRIORITY_REG_PTR) ((UINT32)(lapic_base_address) + PROCESSOR_PRIORITY_REGISTER_OFFSET) )

#define EOI_REGISTER_OFFSET										0xB0
#define EOI_REGISTER_ADDRESS(lapic_base_address)				((EOI_REG_PTR) ((UINT32)(lapic_base_address) + EOI_REGISTER_OFFSET) )

#define LOGICAL_DESTINATION_REGISTER_OFFSET 					0xD0
#define LOGICAL_DESTINATION_REGISTER_ADDRESS(lapic_base_address) ((LOGICAL_DESTINATION_REG_PTR) ((UINT32)(lapic_base_address) + LOGICAL_DESTINATION_REGISTER_OFFSET) )

#define DESTINATION_FORMAT_REGISTER_OFFSET 						0xE0
#define DESTINATION_FORMAT_REGISTER_ADDRESS(lapic_base_address)	((DESTINATION_FORMAT_REG_PTR) ((UINT32)(lapic_base_address) + DESTINATION_FORMAT_REGISTER_OFFSET) )

#define SPURIOUS_INTERRUPT_VECTOR_REGISTER_OFFSET				0xF0
#define SPURIOUS_INTERRUPT_VECTOR_REGISTER_ADDRESS(lapic_base_address)	\
																((volatile SPURIOUS_INTERRUPT_REG_PTR) ((UINT32)(lapic_base_address) + SPURIOUS_INTERRUPT_VECTOR_REGISTER_OFFSET) )
#define IN_SERVICE_REGISTER_OFFSET								0x100
#define IN_SERVICE_REGISTER_ADDRESS(lapic_base_address)			((volatile IN_SERVICE_REG_PTR) ((UINT32)(lapic_base_address) + IN_SERVICE_REGISTER_OFFSET) )

#define TRIGGER_MODE_REGISTER_OFFSET							0x180
#define TRIGGER_MODE_REGISTER_ADDRESS(lapic_base_address)		(volatile TRIGGER_MODE_REG_PTR) ((UINT32)(lapic_base_address) + TRIGGER_MODE_REGISTER_OFFSET) )

#define INTERRUPT_REQUEST_REGISTER_OFFSET						0x200
#define INTERRUPT_REQUEST_REGISTER_ADDRESS(lapic_base_address)	((volatile INTERRUPT_REQUEST_REG_PTR) ((UINT32)(lapic_base_address) + INTERRUPT_REQUEST_REGISTER_OFFSET) )

#define ERROR_STATUS_REGISTER_OFFSET							0x280
#define ERROR_STATUS_REGISTER_ADDRESS(lapic_base_address)		((volatile ERROR_STATUS_REG_PTR) ((UINT32)(lapic_base_address) + ERROR_STATUS_REGISTER_OFFSET) )

#define INTERRUPT_COMMAND_REGISTER_OFFSET_LOW					0x300 /*(0-31 bits)*/
#define INTERRUPT_COMMAND_REGISTER_ADDRESS_LOW(lapic_base_address) 	\
																((volatile INTERRUPT_COMMAND_REGISTER_LOW_PTR)( (UINT32)(lapic_base_address) + INTERRUPT_COMMAND_REGISTER_OFFSET_LOW ))
#define INTERRUPT_COMMAND_REGISTER_OFFSET_HIGH					0x310 /*(0-31 bits)*/
#define INTERRUPT_COMMAND_REGISTER_ADDRESS_HIGH(lapic_base_address) 	\
																((volatile INTERRUPT_COMMAND_REGISTER_HIGH_PTR)( (UINT32)(lapic_base_address) + INTERRUPT_COMMAND_REGISTER_OFFSET_HIGH ))

#define LVT_TIMER_REGISTER_OFFSET 								0x320
#define LVT_TIMER_REGISTER_ADDRESS(lapic_base_address)			((volatile LVT_TIMER_REGISTER_PTR) ((UINT32)(lapic_base_address) + LVT_TIMER_REGISTER_OFFSET) )

#define LVT_THERMAL_SENSOR_REGISTER_OFFSET						0x330
#define LVT_THERMAL_SENSOR_REG_ADDRESS(lapic_base_address)		((volatile LVT_THERMAL_SENSOR_REG_PTR) ((UINT32)(lapic_base_address) + LVT_THERMAL_SENSOR_REGISTER_OFFSET) )

#define LVT_PERF_MON_CNT_REGISTER_OFFSET 						0x340
#define LVT_PERF_MON_CNT_REGISTER_ADDRESS(lapic_base_address)	((volatile LVT_PERFORMANCE_MONITOR_COUNT_REG_PTR) ((UINT32)(lapic_base_address) + LVT_PERF_MON_CNT_REGISTER_OFFSET) )

#define LVT_LINT0_REGISTER_OFFSET 								0x350
#define LVT_LINT0_REGISTER_ADDRESS(lapic_base_address)			((volatile LVT_LINT0_REG_PTR) ((UINT32)(lapic_base_address) + LVT_LINT0_REGISTER_OFFSET) )

#define LVT_LINT1_REGISTER_OFFSET 								0x360
#define LVT_LINT1_REGISTER_ADDRESS(lapic_base_address)			((volatile LVT_LINT1_REG_PTR) ((UINT32)(lapic_base_address) + LVT_LINT1_REGISTER_OFFSET) )

#define LVT_ERROR_REGISTER_OFFSET 								0x370
#define LVT_ERROR_REGISTER_ADDRESS(lapic_base_address)			((volatile LVT_ERROR_REG_PTR) ((UINT32)(lapic_base_address) + LVT_ERROR_REGISTER_OFFSET) )

#define LAPIC_TIMER_INITIAL_COUNT_REGISTER_OFFSET				0x380
#define LAPIC_TIMER_INITIAL_COUNT_REGISTER_ADDRESS(lapic_base_address)	\
																((volatile UINT32*) ((UINT32)(lapic_base_address) + LAPIC_TIMER_INITIAL_COUNT_REGISTER_OFFSET) )
#define LAPIC_TIMER_CURRENT_COUNT_REGISTER_OFFSET				0x390
#define LAPIC_TIMER_CURRENT_COUNT_REGISTER_ADDRESS(lapic_base_address)	\
																((volatile UINT32*) ((UINT32)(lapic_base_address) + LAPIC_TIMER_CURRENT_COUNT_REGISTER_OFFSET) )
#define	LAPIC_TIMER_DIVIDE_REGISTER_OFFSET						0x3E0
#define	LAPIC_TIMER_DIVIDE_REGISTER_ADDRESS(lapic_base_address)	((volatile UINT32*) ((UINT32)(lapic_base_address) + LAPIC_TIMER_DIVIDE_REGISTER_OFFSET) )


#define ACPI_MEMORY_MAP_SIZE									0x400

/*! Initialize LAPIC to receive interrupts.
 *		1) Enable APIC
 *		2) Mask interrupts for LINT0, LINT1, Error, Performance Monitors and Thermal sensors. These registers should be enabled later.
 *		3) Set Task Priority to 0, so that all interrupts will be received
*/
void InitLAPIC()
{
	volatile SPURIOUS_INTERRUPT_REG sir_cmd;
	volatile LVT_TIMER_REGISTER tmr_cmd;
	volatile LVT_PERFORMANCE_MONITOR_COUNT_REG pr_cmd;
	volatile LVT_LINT0_REG lint0_cmd;
	volatile LVT_LINT1_REG lint1_cmd;
	volatile LVT_ERROR_REG er_cmd;
	volatile TASK_PRIORITY_REG tpr_cmd;
	volatile LOCAL_APIC_VERSION_REG ver_cmd;
	volatile DESTINATION_FORMAT_REG dest_cmd;
	
	/*! enable APIC */
	sir_cmd.dword = SPURIOUS_INTERRUPT_VECTOR_REGISTER_ADDRESS(lapic_base_address)->dword;
	sir_cmd.apic_enable = 1;
	sir_cmd.spurious_vector = SPURIOUS_VECTOR_NUMBER;
	SPURIOUS_INTERRUPT_VECTOR_REGISTER_ADDRESS(lapic_base_address)->dword = sir_cmd.dword;

	ver_cmd.dword = LOCAl_APIC_VERSION_REGISTER_ADDRESS(lapic_base_address)->dword;
	
	/*! set destination mode to flat*/
	dest_cmd.dword = DESTINATION_FORMAT_REGISTER_ADDRESS(lapic_base_address)->dword;
	dest_cmd.model = 0xF;
	DESTINATION_FORMAT_REGISTER_ADDRESS(lapic_base_address)->dword = dest_cmd.dword;

	/*! set error register */
	er_cmd.dword = LVT_ERROR_REGISTER_ADDRESS(lapic_base_address)->dword;
	er_cmd.mask = 0;
	er_cmd.vector = ERROR_VECTOR_NUMBER;
	LVT_ERROR_REGISTER_ADDRESS(lapic_base_address)->dword = er_cmd.dword;
	
	/*! set timer register*/
	tmr_cmd.dword = LVT_TIMER_REGISTER_ADDRESS(lapic_base_address)->dword;
	tmr_cmd.mask = 0;
	tmr_cmd.timer_periodic_mode = 0;
	tmr_cmd.vector = LOCAL_TIMER_VECTOR_NUMBER;
	LVT_TIMER_REGISTER_ADDRESS(lapic_base_address)->dword = tmr_cmd.dword;
	
	/*! set LINT0 and LINT1 interrupts */
	lint0_cmd.dword = LVT_LINT0_REGISTER_ADDRESS(lapic_base_address)->dword;
	lint0_cmd.delivery_mode = ICR_DELIVERY_MODE_ExtINT;
	lint0_cmd.mask = 0;
	lint0_cmd.vector = LINT0_VECTOR_NUMBER;
	LVT_LINT0_REGISTER_ADDRESS(lapic_base_address)->dword = lint0_cmd.dword;
	
	lint1_cmd.dword = LVT_LINT1_REGISTER_ADDRESS(lapic_base_address)->dword;
	lint1_cmd.mask = 0;
	lint1_cmd.vector = LINT1_VECTOR_NUMBER;
	LVT_LINT1_REGISTER_ADDRESS(lapic_base_address)->dword = lint1_cmd.dword;
	
	/*! set performance monitoring registers */
	pr_cmd.dword = LVT_PERF_MON_CNT_REGISTER_ADDRESS(lapic_base_address)->dword;
	pr_cmd.mask = 0;
	pr_cmd.vector = PERF_MON_VECTOR_NUMBER;
	LVT_PERF_MON_CNT_REGISTER_ADDRESS(lapic_base_address)->dword = pr_cmd.dword;
	
	/*! set task priority to 0 to receive all interrupts */
	tpr_cmd.dword = TASK_PRIORITY_REGISTER_ADDRESS(lapic_base_address)->dword;
	tpr_cmd.task_priority = 0x0;
	TASK_PRIORITY_REGISTER_ADDRESS(lapic_base_address)->dword = tpr_cmd.dword;
}

/*! Setup the local apic timer
	\param frequency - How many times the timer interrupt should generated per second
	\param periodic - If zero one shot timer else periodic timer
*/
void StartTimer(UINT32 frequency, BYTE periodic)
{
	volatile LVT_TIMER_REGISTER tmr_cmd;
	volatile UINT32 dummy;
	
	/* divide by 1*/
	dummy = *LAPIC_TIMER_DIVIDE_REGISTER_ADDRESS(lapic_base_address);
	*LAPIC_TIMER_DIVIDE_REGISTER_ADDRESS(lapic_base_address) = 0xB;
	
	/* enable timer*/
	tmr_cmd.dword = LVT_TIMER_REGISTER_ADDRESS(lapic_base_address)->dword;
	tmr_cmd.mask = 0;
	tmr_cmd.timer_periodic_mode = (periodic!=0);
	tmr_cmd.vector = LOCAL_TIMER_VECTOR_NUMBER;
	LVT_TIMER_REGISTER_ADDRESS(lapic_base_address)->dword = tmr_cmd.dword;
	
	/* set initial count*/
	dummy = *LAPIC_TIMER_INITIAL_COUNT_REGISTER_ADDRESS(lapic_base_address);
	*LAPIC_TIMER_INITIAL_COUNT_REGISTER_ADDRESS(lapic_base_address) = cpu_frequency / frequency;
	
}

/*! Stops the LAPIC timer
*/
void StopTimer()
{
	volatile LVT_TIMER_REGISTER tmr_cmd;
	/* disable timer*/
	tmr_cmd.dword = LVT_TIMER_REGISTER_ADDRESS(lapic_base_address)->dword;
	tmr_cmd.mask = 1;
	LVT_TIMER_REGISTER_ADDRESS(lapic_base_address)->dword = tmr_cmd.dword;
}

/*! The act of writing anything to this register will cause an EOI to be issued.
 *	\param	int_no	Interrupt number
*/
void SendEndOfInterruptToLapic(int int_no)
{
	UINT32 dummy;
	dummy = EOI_REGISTER_ADDRESS( lapic_base_address )->zero;
	EOI_REGISTER_ADDRESS( lapic_base_address )->zero  = 0;
}

/*!	Start a application processor by issuing IPI messages in the order: INIT, SIPI, SIPI.
 *	\param	apic_id	- apic id of the processor which is to be started.
 *	\param  physical_address - physical address where the boot code resides
 *  \return 0 if the processor is successfully booted.
*/
int StartProcessor(UINT32 apic_id, UINT32 physical_address)
{
	int old_processor_count = count_running_processors;
	UINT8 vector;
	
	/*! We need only first 8 bits(LSB) of the physical address. */
    vector = (physical_address >> 12); 
	
	/*! Issue INIT and then SIPI with proper delay*/
	IssueInterprocessorInterrupt(vector, apic_id, ICR_DELIVERY_MODE_INIT, ICR_DESTINATION_SHORTHAND_NO_SHORTHAND);
	Delay(10); /*! delay for 10 miliseconds*/
	IssueInterprocessorInterrupt(vector, apic_id, ICR_DELIVERY_MODE_SIPI, ICR_DESTINATION_SHORTHAND_NO_SHORTHAND);
	Delay(100); /*! delay 100 milli second*/
	
	/*! Now check if AP has started running? */
	if ( count_running_processors != ( old_processor_count + 1) )
	{
		kprintf("CPU (ACPI Id %d) is not responding to SIPI, marking it as offline\n", apic_id);
		processor[apic_id].state = PROCESSOR_STATE_OFFLINE;
		/*!\todo send shutdown interrupt*/
		return 1;
	}
	
	/*success*/
	return 0;
}

/*! Issues Interprocessor interrupts
 *
 * Provides the following support:
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
void IssueInterprocessorInterrupt(BYTE vector, UINT32 apic_id, ICR_DELIVERY_MODE delivery_mode, ICR_DESTINATION_SHORTHAND destination_shorthand)
{
	INTERRUPT_COMMAND_REGISTER_LOW 	cmd_low;
	INTERRUPT_COMMAND_REGISTER_HIGH cmd_high;
	
	cmd_low.dword = 0;
	cmd_low.vector = vector;
	cmd_low.delivery_mode = delivery_mode;
	cmd_low.destination_mode = ICR_DESTINATION_MODE_PHYSICAL;
	cmd_low.destination_shorthand = destination_shorthand;
	cmd_low.level = ICR_LEVEL_ASSERT;
	
	cmd_high.dword = 0;
	cmd_high.destination_field = apic_id;

	switch(delivery_mode)
	{
		case ICR_DELIVERY_MODE_FIXED:			break;
		case ICR_DELIVERY_MODE_LOWEST_PRIORITY:	break;
		case ICR_DELIVERY_MODE_SMI:				cmd_low.vector = 0; /*! This is for future compatibility as described in specs. */
												break;
		case ICR_DELIVERY_MODE_NMI:				break;
		case ICR_DELIVERY_MODE_INIT:
												cmd_low.trigger_mode = ICR_TRIGGER_MODE_LEVEL;
												cmd_low.vector = 0; /*! This is for future compatibility as described in specs. */
												break;
		case ICR_DELIVERY_MODE_SIPI:			cmd_low.trigger_mode = ICR_TRIGGER_MODE_EDGE;
												break;
		case ICR_DELIVERY_MODE_ExtINT: 			cmd_low.trigger_mode = ICR_TRIGGER_MODE_LEVEL; 
												break;
		case ICR_DELIVERY_MODE_RESERVED:		break;
	}
	
	switch(destination_shorthand)
	{
    	case	ICR_DESTINATION_SHORTHAND_NO_SHORTHAND:			break;
		case	ICR_DESTINATION_SHORTHAND_SELF:					break;
		case	ICR_DESTINATION_SHORTHAND_ALL_INCLUDING_SELF:	cmd_high.destination_field = 0xFF;
																break;
		case	ICR_DESTINATION_SHORTHAND_ALL_EXCLUDING_SELF:	cmd_high.destination_field = 0xFF;
																break;
	}

	/*! Now copy these register contents to actual location of interrupt command register */
	INTERRUPT_COMMAND_REGISTER_ADDRESS_HIGH( lapic_base_address )->dword = cmd_high.dword;
	INTERRUPT_COMMAND_REGISTER_ADDRESS_LOW( lapic_base_address )->dword = cmd_low.dword;
}

/*! \brief Gets the current Interrupt priority level in this processor using LAPIC.
 *  Returns the current interrupt priority level.
 *  Note that lapic is local to each processor.
 */
UINT32 GetInterruptPriorityLevel(void)
{
	TASK_PRIORITY_REG tpr;

	tpr.dword = TASK_PRIORITY_REGISTER_ADDRESS(lapic_base_address)->dword;
	return (UINT32)(tpr.task_priority);
}

/*! \brief Sets the current Interrupt priority level in this processor using LAPIC.
 *  Returns the current interrupt priority level, before changignt o the new value.
 *  Note that lapic is local to each processor.
 */
UINT32 SetInterruptPriorityLevel(UINT32 ipl)
{
	TASK_PRIORITY_REG tpr;
	UINT32 old_ipl;

	tpr.dword = TASK_PRIORITY_REGISTER_ADDRESS(lapic_base_address)->dword;
	old_ipl = (UINT32)(tpr.task_priority);

	tpr.task_priority =  ipl;
	TASK_PRIORITY_REGISTER_ADDRESS(lapic_base_address)->dword = tpr.dword;

	return old_ipl;
}
