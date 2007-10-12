/*!
  \file		idt.c
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created: Wed Oct 10, 2007  04:29PM
  			Last modified: Fri Oct 12, 2007  04:49PM
  \brief	This file contains info on IDT(Interrupt descriptor table).
*/

#include <ace.h>
#include <kernel/i386/idt.h>
#include <string.h>

struct idt_entry idt[IDT_ENTRIES];
struct idt_ptr idtp;

/* Use this function to set an entry in the IDT.*/
void IdtSetGate(BYTE num, UINT32 base)
{
	
	idt[num].base_lo = (base & 0xFFFF);	
	idt[num].base_hi = ((base>>16) & 0xFFFF);
	
	idt[num].sel = 0x08;
	idt[num].always_zero = 0;
	idt[num].flags = 0x8e;
}

/* Installs the IDT */
void IdtInstall()
{
	/* Sets the special IDT pointer up, just like in 'gdt.c' */
	idtp.limit = (sizeof (struct idt_entry) * IDT_ENTRIES) - 1;
	idtp.base = (UINT32)&idt;

	/* Clear out the entire IDT, initializing it to zeros */
	memset(&idt, 0, sizeof(struct idt_entry) * IDT_ENTRIES);

	/* Add any new ISRs to the IDT here using idt_set_gate */
	/* Points the processor's internal register to the new IDT */
	asm volatile ("lidt %0" : :"m"(idtp) );
}
