/*!
  \file		exception.h
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created: Fri Oct 12, 2007  03:15PM
  			Last modified: Fri Oct 12, 2007  04:28PM
  \brief	This file contains declarations and structures pertaining to exceptions, exception handlers.
*/


#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

/* This defines what the stack looks like after an ISR was running */
struct regs
{
    unsigned int gs, fs, es, ds;      /* pushed the segs last */
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pushed by 'pusha' */
    unsigned int int_no, err_code;    /* our 'push byte #' and ecodes do this */
    unsigned int eip, cs, eflags, useresp, ss;   /* pushed by the processor automatically */ 
} __attribute__((packed));

void ExceptionStubInstall();
void ExceptionHandler(struct regs *reg);

#endif
