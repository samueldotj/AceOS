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
	UINT32 cr0, cr1, cr2, cr3;						/*! \todo - pop cr3(page directory register) only if a new task is selected to avoid TLB flush on each interrupt*/
    UINT32 gs, fs, es, ds;     						/*! \todo - remove pushing/poping segment register as it would cause severe performance problem*/
    UINT32 edi, esi, ebp, esp, ebx, edx, ecx, eax;  /*! pusha*/
    UINT32 int_no, error_code;    					/*! manually pushed*/
    UINT32 eip, cs, eflags, useresp, ss;   			/*! pushed by the processor automatically */ 
} __attribute__((packed));

typedef struct regs REGS, *REGS_PTR;

/*! i386 specific Page fault error code*/
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

/*! i386 specific general protection fault error code*/
typedef union gpf_error_code
{
	UINT32 	all;
	struct 
	{
		UINT32 	
			ext:1,						/*if set the exception is because of external event - hardware*/
			idt:1,						/*if set the index portion coming from idt, else it is from gdt/ldt */
			ti:1,						/*if set the index portion coming from ldt, else it is from gdt*/
			segment_selector_index:13,	/*segment / gate selector index*/
			reserved:16;
	};
}GPF_ERROR_CODE, * GPF_ERROR_CODE_PTR;

void SetupExceptionHandlers();
void ExceptionHandler(REGS_PTR reg);

#endif
