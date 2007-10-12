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
extern void ExceptionStub0();
extern void ExceptionStub1();
extern void ExceptionStub2();
extern void ExceptionStub3();
extern void ExceptionStub4();
extern void ExceptionStub5();
extern void ExceptionStub6();
extern void ExceptionStub7();
extern void ExceptionStub8();
extern void ExceptionStub9();
extern void ExceptionStub10();
extern void ExceptionStub11();
extern void ExceptionStub12();
extern void ExceptionStub13();
extern void ExceptionStub14();
extern void ExceptionStub15();
extern void ExceptionStub16();
extern void ExceptionStub17();
extern void ExceptionStub18();
extern void ExceptionStub19();
extern void ExceptionStub20();
extern void ExceptionStub21();
extern void ExceptionStub22();
extern void ExceptionStub23();
extern void ExceptionStub24();
extern void ExceptionStub25();
extern void ExceptionStub26();
extern void ExceptionStub27();
extern void ExceptionStub28();
extern void ExceptionStub29();
extern void ExceptionStub30();
extern void ExceptionStub31();

/*	The first 32 entries in the IDT map to the first 32 ISRs.
*	We set the access flags to 0x8E, which means that the entry is present, is running in ring 0 (kernel level) 
*	and has the lower 5 bits set to the required '14', which is represented by 'E' in hex. 
*/
void ExceptionStubInstall()
{
	IdtSetGate(0,	(unsigned)ExceptionStub0);
	IdtSetGate(1, 	(unsigned)ExceptionStub1);
	IdtSetGate(2, 	(unsigned)ExceptionStub2);
	IdtSetGate(3, 	(unsigned)ExceptionStub3);
	IdtSetGate(4, 	(unsigned)ExceptionStub4);
	IdtSetGate(5, 	(unsigned)ExceptionStub5);
	IdtSetGate(6, 	(unsigned)ExceptionStub6);
	IdtSetGate(7, 	(unsigned)ExceptionStub7);
	IdtSetGate(8, 	(unsigned)ExceptionStub8);
	IdtSetGate(9, 	(unsigned)ExceptionStub9);
	IdtSetGate(10,	(unsigned)ExceptionStub10);
	IdtSetGate(11, 	(unsigned)ExceptionStub11);
	IdtSetGate(12, 	(unsigned)ExceptionStub12);
	IdtSetGate(13, 	(unsigned)ExceptionStub13);
	IdtSetGate(14, 	(unsigned)ExceptionStub14);
	IdtSetGate(15, 	(unsigned)ExceptionStub15);
	IdtSetGate(16, 	(unsigned)ExceptionStub16);
	IdtSetGate(17, 	(unsigned)ExceptionStub17);
	IdtSetGate(18, 	(unsigned)ExceptionStub18);
	IdtSetGate(19, 	(unsigned)ExceptionStub19);
	IdtSetGate(20, 	(unsigned)ExceptionStub20);
	IdtSetGate(21, 	(unsigned)ExceptionStub21);
	IdtSetGate(22, 	(unsigned)ExceptionStub22);
	IdtSetGate(23, 	(unsigned)ExceptionStub23);
	IdtSetGate(24, 	(unsigned)ExceptionStub24);
	IdtSetGate(25, 	(unsigned)ExceptionStub25);
	IdtSetGate(26, 	(unsigned)ExceptionStub26);
	IdtSetGate(27, 	(unsigned)ExceptionStub27);
	IdtSetGate(28, 	(unsigned)ExceptionStub28);
	IdtSetGate(29, 	(unsigned)ExceptionStub29);
	IdtSetGate(30, 	(unsigned)ExceptionStub30);
	IdtSetGate(31, 	(unsigned)ExceptionStub31);
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
	/* Is this a fault whose number is from 0 to 31? */
	if (reg->int_no < 32)
	{
		/* Display the description for the Exception that occurred.
		*  In this tutorial, we will simply halt the system using an
		*  infinite loop 
		*/
		kprintf("Exception!!!!!\n");
		kprintf(exception_messages[reg->int_no]);
		/*now print some useful info from regs structure for debugging*/
		kprintf("\nRegister contents: \n");
		kprintf("gs = 0x%x\tfs = 0x%x\tes = 0x%x\tds = 0x%x\n", reg->gs, reg->fs, reg->es, reg->ds);
		kprintf("edi = 0x%x\tesi = 0x%x\tebp = 0x%x\tesp = 0x%x\n", reg->edi, reg->esi, reg->ebp, reg->esp);
		kprintf("ebx = 0x%x\tedx = 0x%x\tecx = 0x%x\teax = 0x%x\n", reg->ebx, reg->edx, reg->ecx, reg->eax);
		kprintf("eip = 0x%x\tcs = 0x%x\teflags = 0x%x\n", reg->eip, reg->cs, reg->eflags);
		kprintf("useresp = 0x%x\tss = 0x%x\n", reg->useresp, reg->ss);
		kprintf("System Halted!\n");
		ArchHalt();
	}
}
