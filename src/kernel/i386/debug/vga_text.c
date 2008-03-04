/*! \file vga_text.c
    \author Samuel (samueldotj@gmail.com)
    \date 21/09/07 16:55

    \brief This file provide a minimal text out routines. 
    Does direct IO on VGA registers to move text mode cursor
*/
#include <kernel/i386/vga_text.h>
#include <kernel/i386/io.h>
#include <string.h>

#define VGA_TEXT_MEMORY (0xB8000)

#define VGA_CRT_ADDRESS 			0x3D4
#define VGA_CRT_DATA    			0x3D5

#define VGA_CRT_CURSOR_START    	0x0A
#define VGA_CRT_CURSOR_END      	0x0B
#define VGA_CRT_H_START_ADDRESS 	0x0C
#define VGA_CRT_H_END_ADDRESS   	0x0D
#define VGA_CRT_CURSOR_H_LOCATION 	0x0E
#define VGA_CRT_CURSOR_L_LOCATION 	0x0F


/*! \var vga_text_attribute
	This variable holds the vga text attribute(default is 0x07 - white on black).
	It can be changed to set different colors. No lock is gurading the data.
*/
BYTE vga_text_attribute=0x07;

/*! \var vga_tab_col
	This variable specifies number of space should put for a tab character.
	It can be changed to set different colors. No lock is gurading the data.
*/
BYTE vga_tab_col = 4;

/*! \var vga_text_memory
	This variable points where the next text should be written.
*/
BYTE * vga_text_memory = (BYTE *) VGA_TEXT_MEMORY;
BYTE * vga_text_memory_start = (BYTE *)VGA_TEXT_MEMORY;
BYTE * vga_text_memory_end = (BYTE *)(VGA_TEXT_MEMORY + ((VGA_DEFAULT_MAX_COL << 1) * VGA_DEFAULT_MAX_ROW));

BYTE vga_total_col = VGA_DEFAULT_MAX_COL;
BYTE vga_total_row = VGA_DEFAULT_MAX_ROW;
/*! Writes the character to the VGA memory
	Special characters supported \n \r and \t
*/
void VgaPrintCharacter(BYTE c)
{
    switch ( c )
    {
        case  '\n':
            vga_text_memory += (vga_total_col << 1);
            //break;
        case '\r':
    		vga_text_memory -= ((UINT32)(vga_text_memory-vga_text_memory_start) % (vga_total_col << 1) );
            break;
        case '\t':
            vga_text_memory += (vga_tab_col << 1);
            break;
        default:
			*vga_text_memory++ = c;
			*vga_text_memory++ = vga_text_attribute;
    }
    
    if ( vga_text_memory >= vga_text_memory_end )
    	VgaInsertRow();

    VgaMoveCursor( (vga_text_memory-vga_text_memory_start) >> 1 );
}
/*! Internal function used to insert a row in the bottom
*/
void VgaInsertRow()
{
	BYTE * pLastrow;

	memmove ( 
			(BYTE *)VGA_TEXT_MEMORY, 
			(BYTE *)(VGA_TEXT_MEMORY + (vga_total_col << 1)), 
			(vga_total_col << 1) * (vga_total_row-1) 
			);

	pLastrow = vga_text_memory = (BYTE *)(VGA_TEXT_MEMORY + (vga_total_col << 1) * (vga_total_row-1));
	/*cant use memset() because both character and attribute need to be set*/
    while( pLastrow < vga_text_memory_end )
    {
        *pLastrow++ = 0;
        *pLastrow++ = vga_text_attribute;
    }
}
/*! Clears the screen and moves the cursor to 0,0
	May be used during initialization only.
*/
void VgaClearScreen()
{
	BYTE *pVgaText = (BYTE*)VGA_TEXT_MEMORY;
	while( pVgaText < vga_text_memory_end )
    {
        *pVgaText++ = 0;
        *pVgaText++ = vga_text_attribute;
    }
    VgaMoveCursor(0);
}
/*! Moves the cursor the specified position
	Uses VGA registers.
*/
void VgaMoveCursor(UINT16 Offset)
{
    _outp(VGA_CRT_ADDRESS, VGA_CRT_CURSOR_H_LOCATION);
    _outp(VGA_CRT_DATA, Offset>>8);
    _outp(VGA_CRT_ADDRESS, VGA_CRT_CURSOR_L_LOCATION);
    _outp(VGA_CRT_DATA, (Offset<<8)>>8);
}
