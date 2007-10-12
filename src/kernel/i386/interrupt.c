/*!
  \file		interrupt.c
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created: Thu Oct 11, 2007  02:27PM
  			Last modified: Fri Oct 12, 2007  04:30PM
  \brief	This file contains routiens necessary to handle and setup IRQ(Interrupt requests) on the system.	
*/

#include <ace.h>
#include <kernel/i386/idt.h>
#include <kernel/i386/interrupt.h>
#include <kernel/i386/io.h>
#include <kernel/debug.h>


static void InterruptRemap(void);
static void DummyInterruptHandler(struct regs* reg);

/* These are own ISRs that point to our special IRQ handler instead of the regular 'fault_handler' function */
extern void InterruptStub0();
extern void InterruptStub1();
extern void InterruptStub2();
extern void InterruptStub3();
extern void InterruptStub4();
extern void InterruptStub5();
extern void InterruptStub6();
extern void InterruptStub7();
extern void InterruptStub8();
extern void InterruptStub9();
extern void InterruptStub10();
extern void InterruptStub11();
extern void InterruptStub12();
extern void InterruptStub13();
extern void InterruptStub14();
extern void InterruptStub15();

/* This array is actually an array of function pointers. We use this to handle custom IRQ handlers for a given IRQ */
void *interrupt_routines[MAX_INTERRUPTS] ={	0 };

static void DummyInterruptHandler(struct regs* reg)
{
	kprintf("Interrupt recieved from %d\n", reg->int_no);
	return;
}

/*	Define array of function pointers which are the actual interrupt handlers
*	Right now, we only use a dummy interrupt handler which does nothing
*/

/* This installs a custom IRQ handler for the given IRQ */
void InstallInterruptHandler(int interrupt, void (*handler)(struct regs*))
{
	interrupt_routines[interrupt] = handler;
}

/* This clears the handler for a given IRQ */
void UninstallInterruptHandler(int interrupt)
{
	interrupt_routines[interrupt] = 0;
}

/* Normally, IRQs 0 to 7 are mapped to entries 8 to 15. This is a problem in protected mode, because IDT entry 8 is a Double Fault! 
*  Without remapping, every time IRQ0 fires, you get a Double Fault Exception, which is NOT actually what's happening. 
*  We send commands to the Programmable Interrupt Controller (PICs - also called the 8259's) in order to make IRQ0 to 15 be 
*  remapped to IDT entries 32 to 47 
*/
static void InterruptRemap(void)
{
	_outp(0x20, 0x11);
	_outp(0xA0, 0x11);
	_outp(0x21, 0x20);
	_outp(0xA1, 0x28);
	_outp(0x21, 0x04);
	_outp(0xA1, 0x02);
	_outp(0x21, 0x01);
	_outp(0xA1, 0x01);
	_outp(0x21, 0x0);
	_outp(0xA1, 0x0);
}

/* We first remap the interrupt controllers, and then we install the appropriate ISRs to the correct entries in the IDT. 
*  This is just like installing the exception handlers. 
*/
void InterruptInstall()
{
	InterruptRemap();
	InstallInterruptHandler(0, DummyInterruptHandler);	

	IdtSetGate(32, (unsigned)InterruptStub0);
	IdtSetGate(33, (unsigned)InterruptStub1);
	IdtSetGate(34, (unsigned)InterruptStub2);
	IdtSetGate(35, (unsigned)InterruptStub3);
	IdtSetGate(36, (unsigned)InterruptStub4);
	IdtSetGate(37, (unsigned)InterruptStub5);
	IdtSetGate(38, (unsigned)InterruptStub6);
	IdtSetGate(39, (unsigned)InterruptStub7);
	IdtSetGate(40, (unsigned)InterruptStub8);
	IdtSetGate(41, (unsigned)InterruptStub9);
	IdtSetGate(42, (unsigned)InterruptStub10);
	IdtSetGate(43, (unsigned)InterruptStub11);
	IdtSetGate(44, (unsigned)InterruptStub12);
	IdtSetGate(45, (unsigned)InterruptStub13);
	IdtSetGate(46, (unsigned)InterruptStub14);
	IdtSetGate(47, (unsigned)InterruptStub15);
}

/*	All the interrupt stubs call this function.
*	This function calls appropriate interrupt service routines defined in interrupt_routines[].
*	It also returns EOI(End of Interrupt) to the PIC, so that PIC can now get interrupts.
*/
void InterruptHandler(struct regs *reg)
{
	/* This is a blank function pointer */
	void (*handler)(struct regs *reg);

	/* Find out if we have a custom handler to run for this IRQ, and then finally, run it */
	handler = interrupt_routines[reg->int_no - 32];
	if (handler)
	{
    		handler(reg);
	}

	/* If the IDT entry that was invoked was greater than 40 (meaning IRQ8 - 15), then we need to send an EOI to the slave controller */
	if (reg->int_no >= 40)
	{
	    _outp(0xA0, 0x20);
	}

	/* In either case, we need to send an EOI to the master interrupt controller too */
	_outp(0x20, 0x20);
}

