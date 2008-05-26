/*!
  \file		interrupt.c
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created: Thu Oct 11, 2007  02:27PM
  			Last modified: Fri Oct 12, 2007  04:30PM
  \brief	This file contains routiens necessary to handle and setup IRQ(Interrupt requests) on the system.
		
	All the interrupts from 33-48 will be redirected to InterruptHandler().
	InterruptHandler() will dispatch the interrupts from the interrupt_routines[]
	
*/

#include <ace.h>
#include <kernel/i386/idt.h>
#include <kernel/i386/interrupt.h>
#include <kernel/io.h>
#include <kernel/debug.h>


static void SetupPIC(void);
static void DummyInterruptHandler(struct regs* reg);

/* These are own ISRs that point to our special IRQ handler instead of the 
regular 'fault_handler' function */
extern	void 
	InterruptStub0(),	InterruptStub1(),	InterruptStub2(),	InterruptStub3(),
	InterruptStub4(),	InterruptStub5(),	InterruptStub6(),	InterruptStub7(),
	InterruptStub8(),	InterruptStub9(),	InterruptStub10(),	InterruptStub11(),
	InterruptStub12(),	InterruptStub13(),	InterruptStub14(),	InterruptStub15();

/* Array of ISR pointers. We use this to handle custom IRQ handlers for a given IRQ */
void (*interrupt_routines[MAX_INTERRUPTS])(struct regs*);

/* Debug purpose - This dummy handler will just print and exit  */
static void DummyInterruptHandler(struct regs* reg)
{
	kprintf("Interrupt recieved from %d\n", reg->int_no);
}

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

/* We first remap the interrupt controllers, and then we install the appropriate ISRs to the correct entries in the IDT. 
*  This is just like installing the exception handlers. 
*/
void SetupInterruptHandlers()
{
	SetupPIC();
	InstallInterruptHandler(0, DummyInterruptHandler);	

	SetIdtGate(32, (unsigned)InterruptStub0);
	SetIdtGate(33, (unsigned)InterruptStub1);
	SetIdtGate(34, (unsigned)InterruptStub2);
	SetIdtGate(35, (unsigned)InterruptStub3);
	SetIdtGate(36, (unsigned)InterruptStub4);
	SetIdtGate(37, (unsigned)InterruptStub5);
	SetIdtGate(38, (unsigned)InterruptStub6);
	SetIdtGate(39, (unsigned)InterruptStub7);
	SetIdtGate(40, (unsigned)InterruptStub8);
	SetIdtGate(41, (unsigned)InterruptStub9);
	SetIdtGate(42, (unsigned)InterruptStub10);
	SetIdtGate(43, (unsigned)InterruptStub11);
	SetIdtGate(44, (unsigned)InterruptStub12);
	SetIdtGate(45, (unsigned)InterruptStub13);
	SetIdtGate(46, (unsigned)InterruptStub14);
	SetIdtGate(47, (unsigned)InterruptStub15);
}

/*	All the interrupt stubs call this function.
*	This function calls appropriate interrupt service routines defined in interrupt_routines[].
*	It also returns EOI(End of Interrupt) to the PIC, so that PIC can now get interrupts.
*/
void InterruptHandler(struct regs *reg)
{
	/* run the installed handler */
	interrupt_routines[reg->int_no - 32](reg);

	/* If the IDT entry that was invoked was greater than 40 (meaning IRQ8 - 15), then we need to send an EOI to the slave controller */
	if (reg->int_no >= 40)
	{
	    _outp(0xA0, 0x20);
	}

	/* In either case, we need to send an EOI to the master interrupt controller too */
	_outp(0x20, 0x20);
}

/* Normally, IRQs 0 to 7 are mapped to entries 8 to 15. 
*  We send commands to the Programmable Interrupt Controller in order to make IRQ0 to 15 be remapped to IDT entries 32 to 47 
*/
static void SetupPIC(void)
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
