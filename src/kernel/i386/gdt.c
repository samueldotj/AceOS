/*!
  \file		gdt.c
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created: Tue Oct 09, 2007  04:35PM
  			Last modified: Fri Oct 12, 2007  04:52PM
  \brief	This file contains GDT(Global descriptor table) information.
*/

#include <ace.h>
#include <kernel/i386/gdt.h>

/*global descriptor table*/
struct gdt_entry gdt[GDT_ENTRIES] =
{
	/*seglow, baselow, base mid, type, S, DPL, P, seghigh, AVL, 0, D, G, base_high*/
	{     0,	   0,         0,    0, 0,   0, 0,       0,   0, 0, 0, 0,   0},
	{0xFFFF, 	   0,         0,   10, 1,   0, 1,     0xF,   1, 0, 1, 1,   0},  
	{0xFFFF, 	   0,         0,    2, 1,   0, 1,     0xF,   1, 0, 1, 1,   0},
	{0xFFFF, 	   0,         0,   10, 1,   3, 1,     0xF,   1, 0, 1, 1,   0},
	{0xFFFF, 	   0,         0,    2, 1,   3, 1,     0xF,   1, 0, 1, 1,   0}
};

/*global descriptor table register*/
struct gdt_register gp;

/* This will loads new GDT table into processor gdt register */
void LoadGdt()
{
	/* Setup the GDT pointer and limit */
	gp.limit = (sizeof(struct gdt_entry) * GDT_ENTRIES) - 1;
	gp.base = (UINT32)&gdt;

	/* Flush out the old GDT and install the new changes! */
	asm volatile ("lgdt %0" : :"m"(gp) );
	
}
