/*!
  \file		kernel/i386/interrupt.c
  \brief	This file contains routiens necessary to handle and setup IRQ(Interrupt requests) on the system.
		
	All the interrupts from 33-48 will be redirected to InterruptHandler().
	InterruptHandler() will dispatch the interrupts from the interrupt_routines[]
*/

#include <ace.h>
#include <kernel/i386/idt.h>
#include <kernel/io.h>
#include <kernel/debug.h>
#include <kernel/apic.h>

/* These are own ISRs that point to our special IRQ handler instead of the 
regular 'fault_handler' function */
extern	void 
	InterruptStub0(),	InterruptStub1(),	InterruptStub2(),	InterruptStub3(),
	InterruptStub4(),	InterruptStub5(),	InterruptStub6(),	InterruptStub7(),
	InterruptStub8(),	InterruptStub9(),	InterruptStub10(),	InterruptStub11(),
	InterruptStub12(),	InterruptStub13(),	InterruptStub14(),	InterruptStub15();

/* We first remap the interrupt controllers, and then we install the appropriate ISRs to the correct entries in the IDT. 
*  This is just like installing the exception handlers. 
*/
void SetupInterruptStubs()
{
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
