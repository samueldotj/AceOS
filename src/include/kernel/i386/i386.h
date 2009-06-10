/*! 
	\file	kernel/i386/i386.h
	\brief	i386 architecture specific structure and function declarations
*/

#ifndef I386_H
#define I386_H

#include <ace.h>
#include <kernel/i386/vga_text.h>
#include <kernel/i386/gdt.h>
#include <kernel/i386/idt.h>
#include <kernel/i386/exception.h>
#include <kernel/i386/pmem.h>
#include <kernel/i386/cpuid.h>
#include <kernel/i386/apic.h>
#include <kernel/i386/ioapic.h>
#include <kernel/i386/processor.h>

/*! various eflag register values */
#define EFLAG_RESERVED_BIT	( 1<<1 )
#define EFLAG_VM86			( 1<<17 )
#define EFLAG_VIF			( 1<<19 )
#define EFLAG_IF			( 1<<9  )
#define EFLAG_IOPL0			( 0 )
#define EFLAG_IOPL1			( 1<<12 )
#define EFLAG_IOPL2			( 2<<12 )
#define EFLAG_IOPL3			( 3<<12 )

/*! task initial eflag value */
#define EFLAG_VALUE			( EFLAG_RESERVED_BIT | EFLAG_IF | EFLAG_IOPL3 )

/* segment:offset pair */
typedef UINT32 FARPTR;

/* Make a FARPTR from a segment and an offset */
#define MK_FP(seg, off)    ((FARPTR) (((UINT32) (seg) << 16) | (UINT16) (off)))
/* Extract the segment part of a FARPTR */
#define FP_SEG(fp)        (((FARPTR) fp) >> 16)
/* Extract the offset part of a FARPTR */
#define FP_OFF(fp)        (((FARPTR) fp) & 0xffff)
/* Convert a segment:offset pair to a linear address */
#define FP_TO_LINEAR(seg, off) ((void*) ((((UINT16) (seg)) << 4) + ((UINT16) (off))))

typedef struct thread_i386
{
	UINT32			is_v86:1,							/*! set if this thread is a v86 mode thread*/
					eflag_if:1;							/*! interrupt enable bit in eflag - updated only for v86*/
	
	void 			(*interrupt_handler)(REGS_PTR reg);	/*! v86 interrupt handler for this thread*/
	
	REGS_V86		input_regs;							/*! input values passed to this thread*/
}THREAD_I386, * THREAD_I386_PTR;

UINT32 I386LinearToFp(UINT32 ptr);
REGS_V86_PTR CallBiosIsr(BYTE interrupt, REGS_V86_PTR input);

#endif
