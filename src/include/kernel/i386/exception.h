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

#include <ace.h>

/* This defines what the stack looks like after an ISR was running */
struct regs
{
	UINT32 cr0, cr1, cr2, cr3;
    UINT32 gs, fs, es, ds;      /* pushed the segs last */
    UINT32 edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pushed by 'pusha' */
    UINT32 int_no, err_code;    /* our 'push byte #' and ecodes do this */
    UINT32 eip, cs, eflags, useresp, ss;   /* pushed by the processor automatically */ 
} __attribute__((packed));

typedef struct regs REGS, *REGS_PTR;

void SetupExceptionHandlers();
void ExceptionHandler(REGS_PTR reg);

#endif
