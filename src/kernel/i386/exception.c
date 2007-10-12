/*!
  \file		ExceptionStub.c
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
	ExceptionStub12(), 	ExceptionStub13(), 	ExceptionStub14(),  ExceptionStub15(),
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
	SetIdtGate(14, 	(unsigned)ExceptionStub14);
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


/* All of our Exception handling Interrupt Service Routines will point to this function. 
*  This will tell us what exception has happened. 
*/
void ExceptionHandler(struct regs *reg)
{
	kprintf("%s - Exception No [%d]\n", exception_messages[reg->int_no], reg->int_no);
	/*now print some useful info from regs structure for debugging*/
	kprintf("gs = 0x%x fs = 0x%x\tes = 0x%x\tds = 0x%x\n", reg->gs, reg->fs, reg->es, reg->ds);
	kprintf("edi = 0x%x\tesi = 0x%x\tebp = 0x%x\tesp = 0x%x\n", reg->edi, reg->esi, reg->ebp, reg->esp);
	kprintf("ebx = 0x%x\tedx = 0x%x\tecx = 0x%x\teax = 0x%x\n", reg->ebx, reg->edx, reg->ecx, reg->eax);
	kprintf("eip = 0x%x\tcs = 0x%x\teflags = 0x%x\n", reg->eip, reg->cs, reg->eflags);
	kprintf("useresp = 0x%x\tss = 0x%x\n", reg->useresp, reg->ss);
	kprintf("System Halted!\n");
	ArchHalt();
}
