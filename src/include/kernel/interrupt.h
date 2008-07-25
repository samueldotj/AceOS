/*!
  \file		interrupt.h
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created: Fri Oct 12, 2007  03:36PM
  			Last modified: Fri Oct 12, 2007  04:29PM
  \brief	This file contains declarations pertaining to interrupts.
*/


#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#if	ARCH == i386
	#include <kernel/i386/exception.h>
	#define MAX_INTERRUPTS	256
#endif

typedef struct interrupt_info
{
	int	interrupt_number;
	int interrupt_type;
	int interrupt_priority;
	int	device_number;
}INTERRUPT_INFO, * INTERRUPT_INFO_PTR;

typedef enum isr_return_code
{
	ISR_CONTINUE_PROCESSING=0,
	ISR_END_PROCESSING,
	ISR_ERROR
}ISR_RETURN_CODE;

typedef ISR_RETURN_CODE (*ISR_HANDLER) (INTERRUPT_INFO_PTR interrupt_info, void * arg);

void InstallInterruptHandler(int interrupt_number, ISR_HANDLER isr_handler, void * custom_argument);
void UninstallInterruptHandler(int interrupt_number, ISR_HANDLER isr_handler);

#endif
