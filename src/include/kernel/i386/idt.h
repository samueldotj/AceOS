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
#define MAX_INTERRUPTS 16

/* This exists in 'start.asm', and is used to load our IDT */
extern void IdtLoad();

/* Defines an IDT entry */
struct idt_entry
{
    UINT16 base_lo;
    UINT16 sel;        /* Our kernel segment goes here! */
    BYTE always_zero;     /* This will ALWAYS be set to 0! */
    BYTE flags;       /* Set using the above table! */
    UINT16 base_hi;
} __attribute__((packed));

struct idt_ptr
{
    UINT16 limit;
    UINT32 base;
} __attribute__((packed));

/* Declare an IDT of IDT_ENTRIES(256) entries. Although we will only use the first 32 entries, the rest exists as a bit of a trap. 
 * If any undefined IDT entry is hit, it normally will cause an "Unhandled Interrupt" exception. 
 * Any descriptor for which the 'presence' bit is cleared (0) will generate an "Unhandled Interrupt" exception 
*/

void IdtInstall();
void IdtSetGate(BYTE num, UINT32 base);

#endif

