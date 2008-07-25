/*!
  \file		idt.h
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created: Wed Oct 10, 2007  04:25PM
  			Last modified: Fri Oct 12, 2007  03:12PM
  \brief	
*/
#ifndef _IDT_H_
#define _IDT_H_

#include <ace.h>


#define IDT_ENTRIES 256

/* Defines an IDT entry */
struct idt_entry
{
    UINT16 base_low;
    UINT16 selector;
    BYTE always_zero; 
    BYTE flags;
    UINT16 base_high;
} __attribute__((packed));

struct idt_ptr
{
    UINT16 limit;
    UINT32 base;
} __attribute__((packed));

void LoadIdt();
void SetIdtGate(BYTE num, UINT32 base);

#endif

