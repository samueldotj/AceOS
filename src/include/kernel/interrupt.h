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

extern INTERRUPT_HANDLER interrupt_handlers[MAX_INTERRUPTS];

void InstallInterruptHandler(int interrupt_number, ISR_HANDLER isr_handler, void * custom_argument);
void UninstallInterruptHandler(int interrupt_number, ISR_HANDLER isr_handler);

#endif
