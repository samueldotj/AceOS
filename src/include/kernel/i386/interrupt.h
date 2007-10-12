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

#include <kernel/i386/exception.h>

void InstallInterruptHandler(int interrupt, void (*handler)(struct regs*));
void UninstallInterruptHandler(int interrupt);
void SetupInterruptHandlers();


#endif
