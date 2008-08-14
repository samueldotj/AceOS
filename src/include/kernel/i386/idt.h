/*!
  \file		kernel/i386/idt.h
  \brief	Interrupt Descriptor Table related data structures and functions
*/
#ifndef _IDT_H_
#define _IDT_H_

#include <ace.h>

/*! Total interrupt descriptor table entries in i386*/
#define IDT_ENTRIES 256

/*! Interrupt Descriptor Entry data structure*/
struct idt_entry
{
    UINT16 base_low;
    UINT16 selector;
    BYTE always_zero; 
    BYTE flags;
    UINT16 base_high;
} __attribute__((packed));

/*! Interrupt Descriptor Entry pointer*/
struct idt_ptr
{
    UINT16 limit;
    UINT32 base;
} __attribute__((packed));

void LoadIdt();
void SetIdtGate(BYTE num, UINT32 base);

#endif

