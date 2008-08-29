/*!
  \file		kernel/i386/exception.h
  \brief	This file contains declarations and structures pertaining to exceptions, exception handlers.
*/


#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include <ace.h>

/*! Exception Frame - Stack contains the following values when a exception or interrupts are raised */
struct regs
{
	UINT32 cr0, cr1, cr2, cr3;
    UINT32 gs, fs, es, ds;      /* pushed the segs last */
    UINT32 edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pushed by 'pusha' */
    UINT32 int_no, error_code;    /* our 'push byte #' and ecodes do this */
    UINT32 eip, cs, eflags, useresp, ss;   /* pushed by the processor automatically */ 
} __attribute__((packed));

typedef struct regs REGS, *REGS_PTR;

/*! Page fault error code*/
typedef union pf_error_code
{
	UINT32 	all;
	struct 
	{
		UINT32 	
			present:1,
			write:1,
			user:1,
			rsvd:1,
			reserved:28;
	};
}PF_ERROR_CODE, * PF_ERROR_CODE_PTR;

void SetupExceptionHandlers();
void ExceptionHandler(REGS_PTR reg);

#endif
