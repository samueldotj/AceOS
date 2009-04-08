/*!
  \file		kernel/i386/idt.h
  \brief	Interrupt Descriptor Table related data structures and functions
*/
#ifndef _IDT_H_
#define _IDT_H_

#include <ace.h>
#include <kernel/i386/gdt.h>

/*! Total interrupt descriptor table entries in i386*/
#define IDT_ENTRIES 256

#define	IDT_TYPE_TASK_GATE			5
#define	IDT_TYPE_INTERRUPT_GATE		14
#define	IDT_TYPE_TRAP_GATE			15

/*! Interrupt Descriptor Entry data structure*/
struct idt_entry
{
    UINT16 	base_low;								/*base address of interrupt handler*/
    UINT16 	selector;								/*segment selector for the interrupt handler - always KERNEL_CODE_SELECTOR */
    BYTE 	always_zero; 							/*reserved and always zero*/
    BYTE 	type:5,									/*type of the interrupt descriptor*/
			descriptor_privilege_level:2,			/*privilege level*/
			present:1;
    UINT16 base_high;								/*base address of interrupt handler*/
} __attribute__((packed));

/*! Interrupt Descriptor Entry pointer*/
struct idt_ptr
{
    UINT16 limit;
    UINT32 base;
} __attribute__((packed));

void LoadIdt();
void SetIdtGate(BYTE num, UINT32 base, BYTE type, BYTE dpl);
void SetIdtTaskGate(BYTE num, BYTE task_selector, BYTE dpl);

#endif


