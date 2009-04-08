/*! 
	\file	kernel/i386/tss.h
	\brief  Task state segment related data structure definitions and functions
*/

#ifndef TSS_H
#define TSS_H

#include <ace.h>

struct tss{
	UINT16	link;
	UINT16	reserved_1;

	UINT32	esp0;
	UINT16	ss0;
	UINT16	reserved_2;

	UINT32	esp1;
	UINT16	ss1;
	UINT16	reserved_3;

	UINT32	esp2;
	UINT16	ss2;
	UINT16	reserved_4;

	UINT32	cr3;
	UINT32	eip;
	UINT32	eflags;

	UINT32	eax;
	UINT32	ecx;
	UINT32	edx;
	UINT32	ebx;

	UINT32	esp;
	UINT32	ebp;

	UINT32	esi;
	UINT32	edi;

	UINT16	es;
	UINT16	reserved_6;

	UINT16	cs;
	UINT16	reserved_7;

	UINT16	ss;
	UINT16	reserved_8;

	UINT16	ds;
	UINT16	reserved_9;

	UINT16	fs;
	UINT16	reserved_10;

	UINT16	gs;
	UINT16	reserved_11;

	UINT16	ldt;
	UINT16	reserved_12;

	UINT16	trap;
	UINT16	iomap;
}__attribute__ ((aligned (8), packed));

typedef volatile struct tss TSS, * TSS_PTR;

void LoadTss();
void FillTssForDoubleFaultHandler( void * fault_handler, UINT32 kernel_stack );

#endif
