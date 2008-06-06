/*!
  \file		exception.c
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created: Wed Oct 10, 2007  05:21PM
  			Last modified: Fri Oct 12, 2007  04:27PM
  \brief	
*/

#include <ace.h>
#include <kernel/i386/idt.h>
#include <kernel/i386/exception.h>
#include <kernel/debug.h>
#include <kernel/arch.h>

/* These are function prototypes for all of the exception
*  handlers: The first 32 entries in the IDT are reserved
*  by Intel, and are designed to service exceptions! */
extern	void 
	ExceptionStub0(), 	ExceptionStub1(), 	ExceptionStub2(), 	ExceptionStub3(),
	ExceptionStub4(), 	ExceptionStub5(), 	ExceptionStub6(), 	ExceptionStub7(),
	ExceptionStub8(), 	ExceptionStub9(), 	ExceptionStub10(), 	ExceptionStub11(),
	ExceptionStub12(), 	ExceptionStub13(), 	PageFaultHandlerStub(),  ExceptionStub15(),
	ExceptionStub16(), 	ExceptionStub17(),	ExceptionStub18(),	ExceptionStub19(),
	ExceptionStub20(), 	ExceptionStub21(),	ExceptionStub22(), 	ExceptionStub23(),
	ExceptionStub24(),	ExceptionStub25(),	ExceptionStub26(),	ExceptionStub27(),
	ExceptionStub28(),	ExceptionStub29(),	ExceptionStub30(),	ExceptionStub31();


/*	The first 32 entries in the IDT map to the first 32 ISRs.
*	We set the access flags to 0x8E, which means that the entry is present, is running in ring 0 (kernel level) 
*	and has the lower 5 bits set to the required '14', which is represented by 'E' in hex. 
*/
void SetupExceptionHandlers()
{
	SetIdtGate(0,	(unsigned)ExceptionStub0);
	SetIdtGate(1, 	(unsigned)ExceptionStub1);
	SetIdtGate(2, 	(unsigned)ExceptionStub2);
	SetIdtGate(3, 	(unsigned)ExceptionStub3);
	SetIdtGate(4, 	(unsigned)ExceptionStub4);
	SetIdtGate(5, 	(unsigned)ExceptionStub5);
	SetIdtGate(6, 	(unsigned)ExceptionStub6);
	SetIdtGate(7, 	(unsigned)ExceptionStub7);
	SetIdtGate(8, 	(unsigned)ExceptionStub8);
	SetIdtGate(9, 	(unsigned)ExceptionStub9);
	SetIdtGate(10,	(unsigned)ExceptionStub10);
	SetIdtGate(11, 	(unsigned)ExceptionStub11);
	SetIdtGate(12, 	(unsigned)ExceptionStub12);
	SetIdtGate(13, 	(unsigned)ExceptionStub13);
	SetIdtGate(14, 	(unsigned)PageFaultHandlerStub);
	SetIdtGate(15, 	(unsigned)ExceptionStub15);
	SetIdtGate(16, 	(unsigned)ExceptionStub16);
	SetIdtGate(17, 	(unsigned)ExceptionStub17);
	SetIdtGate(18, 	(unsigned)ExceptionStub18);
	SetIdtGate(19, 	(unsigned)ExceptionStub19);
	SetIdtGate(20, 	(unsigned)ExceptionStub20);
	SetIdtGate(21, 	(unsigned)ExceptionStub21);
	SetIdtGate(22, 	(unsigned)ExceptionStub22);
	SetIdtGate(23, 	(unsigned)ExceptionStub23);
	SetIdtGate(24, 	(unsigned)ExceptionStub24);
	SetIdtGate(25, 	(unsigned)ExceptionStub25);
	SetIdtGate(26, 	(unsigned)ExceptionStub26);
	SetIdtGate(27, 	(unsigned)ExceptionStub27);
	SetIdtGate(28, 	(unsigned)ExceptionStub28);
	SetIdtGate(29, 	(unsigned)ExceptionStub29);
	SetIdtGate(30, 	(unsigned)ExceptionStub30);
	SetIdtGate(31, 	(unsigned)ExceptionStub31);
}

/*	This is a simple string array. It contains the message that corresponds to each and every exception. 
*	We get the correct message by accessing like:  exception_message[interrupt_number] 
*/
char *exception_messages[] =
{
	"Division By Zero",
	"Debug",
	"Non Maskable Interrupt",
	"Breakpoint",
	"Into Detected Overflow",
	"Out of Bounds",
	"Invalid Opcode",			
	"No Coprocessor",			
	"Double Fault",
	"Coprocessor Segment Overrun",
	"Bad TSS",
	"Segment Not Present",
	"Stack Fault",
	"General Protection Fault",
	"Page Fault",
	"Unknown Interrupt",
	"Coprocessor Fault",
	"Alignment Check",
	"Machine Check",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved"
};

#define REG_FORMAT	"0x%08x\t"
#define SEG_FORMAT	"0x%02x"

#define PRINT_REGS(reg)\
	kprintf("eax="REG_FORMAT"ebx="REG_FORMAT"ecx="REG_FORMAT"edx="REG_FORMAT"\n", reg->eax, reg->ebx, reg->ecx, reg->edx);\
	kprintf("esi="REG_FORMAT"edi="REG_FORMAT"ebp="REG_FORMAT"esp="REG_FORMAT"\n", reg->esi, reg->edi, reg->ebp, reg->esp);\
	kprintf("cs:eip="SEG_FORMAT":"REG_FORMAT"[user stack ss:esp="SEG_FORMAT":"REG_FORMAT"]\n", reg->cs, reg->eip, reg->ss, reg->useresp);\
	kprintf("ds="SEG_FORMAT" es="SEG_FORMAT" gs="SEG_FORMAT" fs="SEG_FORMAT" eflags=0x%X\n", reg->ds, reg->es, reg->gs, reg->fs, reg->eflags);\
	kprintf("cr0="REG_FORMAT" cr1="REG_FORMAT" cr2="REG_FORMAT" cr3="REG_FORMAT"\n", reg->cr0, reg->cr1, reg->cr2, reg->cr3);

/* All of our Exception handling Interrupt Service Routines will point to this function. 
*  This will tell us what exception has happened. 
*/
void ExceptionHandler(REGS_PTR reg)
{
	kprintf("######### Unhandled Exception [%s] - No [%d]########\n", exception_messages[reg->int_no], reg->int_no);
	/*now print some useful info from regs structure for debugging*/
	PRINT_REGS( reg );
	kprintf("System Halted!\n");
	ArchHalt();
}

void PageFaultHandler(REGS_PTR reg)
{
	PF_ERROR_CODE err;
	err.all = reg->error_code;
	if ( err._.rsvd ) 
		kprintf( "Page fault - RESERVED BIT SET\n");
	else
		kprintf("Page fault(code %d): %s page %s attempt and %s fault.\n",  
			reg->error_code,
			err._.user ? "User" : "Supervisor" ,
			err._.write ? "write" : "read",
			err._.present ? "protection" : "page not present");
			
	PRINT_REGS(reg);
	ArchHalt();
}
