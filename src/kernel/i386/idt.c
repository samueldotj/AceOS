/*!
  \file		idt.c
  \brief	This file contains info on IDT(Interrupt descriptor table).
*/

#include <ace.h>
#include <kernel/i386/gdt.h>
#include <kernel/i386/idt.h>
#include <string.h>


/*System Interrupt Descriptor Table*/
struct idt_entry idt[IDT_ENTRIES];

/*Interrupt Descriptor Table Register*/
struct idt_ptr idtp;

/* Use this function to set an entry in the IDT.*/
void SetIdtGate(BYTE num, UINT32 base)
{	
	idt[num].base_low = (base & 0xFFFF);	
	idt[num].base_high = ((base>>16) & 0xFFFF);
	
	idt[num].selector = KERNEL_CODE_SELECTOR;
	idt[num].always_zero = 0;
	idt[num].flags = 0x8e;
}

/* Loads the Interrupt Descriptor Table*/
void LoadIdt()
{
	/* Sets the special IDT pointer up, just like in 'gdt.c' */
	idtp.limit = (sizeof (struct idt_entry) * IDT_ENTRIES) - 1;
	idtp.base = (UINT32)&idt;

	/* Clear out the entire IDT, initializing it to zeros */
	memset(&idt, 0, sizeof(struct idt_entry) * IDT_ENTRIES);
	
	/*load idtr*/
	asm volatile ("lidt %0" : :"m"(idtp) );
}
