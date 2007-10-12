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

/* Our GDT, with 3 entries, and finally our special GDT pointer */
struct gdt_entry gdt[3];
struct gdt_register gp;

/* This will be a function in start.asm. We use this to properly
 * reload the new segment registers.
*/
extern void GdtFlush();

static void GdtSetGate(int num, UINT32 base, UINT32 limit, BYTE access, BYTE gran);


/* Setup a descriptor in the Global Descriptor Table */
static void GdtSetGate(int num, UINT32 base, UINT32 limit, BYTE access, BYTE gran)
{
	/* Setup the descriptor base address */
	gdt[num].base_low = (base & 0xFFFF);
	gdt[num].base_middle = (base >> 16) & 0xFF;
	gdt[num].base_high = (base >> 24) & 0xFF;

	/* Setup the descriptor limits */
	gdt[num].limit = (limit & 0xFFFF);
	gdt[num].granularity = ((limit >> 16) & 0x0F);

	/* Finally, set up the granularity and access flags */
	gdt[num].granularity |= (gran & 0xF0);
	gdt[num].access = access;
}

/* Should be called by main. This will setup the special GDT
*  pointer, set up the first 3 entries in our GDT, and then
*  finally call gdt_flush() in our assembler file in order
*  to tell the processor where the new GDT is and update the
*  new segment registers */
void GdtInstall()
{
	/* Setup the GDT pointer and limit */
	gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
	gp.base = (UINT32)&gdt;

	/* Our NULL descriptor */
	GdtSetGate(0, 0, 0, 0, 0);

	/* The second entry is our Code Segment. The base address
	*  is 0, the limit is 4GBytes, it uses 4KByte granularity,
	*  uses 32-bit opcodes, and is a Code Segment descriptor.
	*  Please check the GDT table in order
	*  to see exactly what each value means */
	GdtSetGate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

	/* The third entry is our Data Segment. It's EXACTLY the
	*  same as our code segment, but the descriptor type in
	*  this entry's access byte says it's a Data Segment */
	GdtSetGate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

	/* Flush out the old GDT and install the new changes! */
	asm volatile ("lgdt %0" : :"m"(gp) );
}
