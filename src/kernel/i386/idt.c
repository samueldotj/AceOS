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

/*! Sets an entry in interrupt descriptor table as interrupt gate
	\param	num - Interrupt vector number
	\param  base - Address of the interrrupt handler
	\param	type - interrupt descriptor type(task gate, interrupt gate or trap gate)
	\param 	dpl - descriptor privilege level (0 or 3)
*/
void SetIdtGate(BYTE num, UINT32 base, BYTE type, BYTE dpl)
{	
	idt[num].base_low = (base & 0xFFFF);	
	idt[num].base_high = ((base>>16) & 0xFFFF);
	
	idt[num].selector = KERNEL_CODE_SELECTOR;
	idt[num].always_zero = 0;
	idt[num].present = 1;
	idt[num].type = type;
	idt[num].descriptor_privilege_level = dpl;
}

/*! Sets an entry in interrupt descriptor table as task gate
	\param	num - Interrupt vector number
	\param  task_selector - task selector in the gdt
	\param 	dpl - descriptor privilege level (0 or 3)
*/
void SetIdtTaskGate(BYTE num, BYTE task_selector, BYTE dpl)
{
	idt[num].base_low = 0;	
	idt[num].base_high = 0;
	
	idt[num].selector = task_selector;
	idt[num].always_zero = 0;
	idt[num].present = 1;
	idt[num].type = 0x5;
	idt[num].descriptor_privilege_level = dpl;
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
