/*!
  \file		interrupt.h
  \brief	This file contains declarations pertaining to interrupts.
*/


#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#include <ds/list.h>
#if	ARCH == i386
	#include <kernel/i386/exception.h>
	#define MAX_INTERRUPTS	256
#endif

typedef struct interrupt_info
{
	int	interrupt_number;			/*! interrupt number*/
	int interrupt_type;				/*! Type of the interrupt //edge, level, etc*/
	int interrupt_priority;			/*! Priority of the interrupt*/
	int	device_number;				/*! Device number which generated the interrupt*/

#if	ARCH == i386
	REGS_PTR	regs;				/*! Register contents at the time of interrupt*/
#endif 
}INTERRUPT_INFO, * INTERRUPT_INFO_PTR;

typedef enum isr_return_code
{
	ISR_CONTINUE_PROCESSING=0,
	ISR_END_PROCESSING,
	ISR_ERROR
}ISR_RETURN_CODE;

typedef ISR_RETURN_CODE (*ISR_HANDLER) (INTERRUPT_INFO_PTR interrupt_info, void * arg);

/*! Interrupt handler internal list
this structure is not used by anyother module so declared inside this c file*/
typedef struct interrupt_handler
{
	ISR_HANDLER 	isr;			//interrrupt service routine for this irq
	void *			isr_argument;	//custom data registered by the isr
	
	LIST			next_isr;		//next isr sharing the same interrupt line
}INTERRUPT_HANDLER, *INTERRUPT_HANDLER_PTR;


/*! A list of Interrupt priority levels maintained by ACE.
 *  Higher the priority number, lower the value,level.
 *  Ex: Panic is the highest level(0).
 */
typedef enum irq_priority_levels
{
	IRQ_PRIORITY_LEVELS_PANIC, /* panic */
	IRQ_PRIORITY_LEVELS_MACHINE_CHECK, /* Machine check */
	IRQ_PRIORITY_LEVELS_TIMER,  /* timer */
	IRQ_PRIORITY_LEVELS_SCHEDULER, /* scheduler */
	IRQ_PRIORITY_LEVELS_IO, /* I/0 , device drivers top half */
	IRQ_PRIORITY_LEVELS_GENERIC /* generic code */
}IRQ_PRIORITY_LEVELS;


extern INTERRUPT_HANDLER interrupt_handlers[MAX_INTERRUPTS];

void InstallInterruptHandler(int interrupt_number, ISR_HANDLER isr_handler, void * custom_argument);
void UninstallInterruptHandler(int interrupt_number, ISR_HANDLER isr_handler);

void SendEndOfInterrupt(int int_no);

IRQ_PRIORITY_LEVELS RaiseInterruptPriorityLevel(IRQ_PRIORITY_LEVELS ipl);
void RestoreInterruptPriorityLevel(IRQ_PRIORITY_LEVELS level);

UINT32 SetInterruptPriorityLevel(UINT32 ipl);
UINT32 GetInterruptPriorityLevel(void);

#endif
