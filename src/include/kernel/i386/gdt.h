/*! \file       gdt.h
        \author DilipSimha N M
        \date   09/10/2007 23:20
        \brief  GDT related declarations and structure definitions
*/

#ifndef GDT_H
#define GDT_H

/*! GDT structure.
 * Defines a GDT entry. We say packed, because it prevents the compiler from doing things that it thinks is best.
 * Prevent compiler "optimization" by packing.
*/
struct gdt_entry
{
        UINT16 limit;
        UINT16 base_low;  /*0-15*/
        BYTE base_middle; /*16-23*/
        BYTE access;  /*!< P(1bit) | DPL(2bit) | S(1bit) | Type(4bit)*/
        BYTE granularity; /*!< G(1bit) | D/B(1bit) | 0(1bit) | AVL(1bit) | limit16_19(4bit)*/
        BYTE base_high;   /*24-31*/
} __attribute__ ((packed));

/* P=Present, DPL=Descriptor Privilege Level, S=Descriptor type (0=system; 1=code or data),
 * Type=Segment type, G=Granularity, D/B=Default operation size(0=16bit; 1=32bit segment),
 * AVL=Available for use by system software.
*/


/*! The GDT register format.
 *  Special pointer which includes the limit: The max bytes taken up by the GDT, minus 1.
        Again, this NEEDS to be packed
*/
struct gdt_register
{
        UINT16 limit;
        UINT32 base;
} __attribute__ ((packed));


void GdtInstall();

#endif
