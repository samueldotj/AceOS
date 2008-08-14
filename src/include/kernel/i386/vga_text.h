/*! \file	kernel/i386/vga_text.h
	\brief	Kernel VGA Console Routines
	This file provide a minimal text out routines.
*/
#ifndef VGA_TEXT__H
#define VGA_TEXT__H

#include <ace.h>

extern BYTE vga_text_attribute;
extern BYTE vga_tab_col;

#define VGA_DEFAULT_MAX_ROW	25
#define VGA_DEFAULT_MAX_COL 80

#ifdef __cplusplus
	extern "C" {
#endif

void VgaPrintCharacter(BYTE c);
void VgaInsertRow();
void VgaClearScreen();
void VgaMoveCursor(UINT16 Offset);

#ifdef __cplusplus
	}
#endif

#endif

