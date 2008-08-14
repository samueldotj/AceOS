/*! 
	\file	kernel/i386/gdt.h
	\brief  Global Descriptor Table related data structure definitions and functions
*/

#ifndef GDT_H
#define GDT_H

#define KERNEL_CODE_SELECTOR	8
#define KERNEL_DATA_SELECTOR	16

#define USER_CODE_SELECTOR		24
#define USER_DATA_SELECTOR		32

/*! GDT structure.
 * Defines a GDT entry. We say packed, because it prevents the compiler from doing things that it thinks is best.
 * Prevent compiler "optimization" by packing.
*/
struct gdt_entry
{
	UINT32 
		segment_limit_low:16,
		base_low:16;

	UINT32
		base_mid:8,
		type:4,
		system:1,
		descriptor_privilege_level:2,
		present:1,

		segment_limit_high:4,
		software:1,
		reserved:1,
		default_operation_size:1,
		granularity:1,
		base_high:8;
}__attribute__ ((aligned (8), packed));


/*! The GDT register format.
 *  Special pointer which includes the limit: The max bytes taken up by the GDT, minus 1.
        Again, this NEEDS to be packed
*/
struct gdt_register
{
        UINT16 limit;
        UINT32 base;
} __attribute__ ((packed));

#define GDT_ENTRIES		5

/*global descriptor table*/
extern struct gdt_entry gdt[GDT_ENTRIES];

void LoadGdt();

#endif
