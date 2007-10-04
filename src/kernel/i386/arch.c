/*!
	\file		arch.c
	\author		Samuel
	\version 	3.0
	\date	
  			Created: 26/09/07 15:21
  			Last modified: 26/09/07 15:21
	\brief	contains architecture related interface routines.
*/
#include <kernel/debug.h>
#include <kernel/gdb.h>
#include <kernel/i386/vga_text.h>

/*! This is the startup module for i386 architecture
	This should initialize all the i386 specific data/variables
*/
void ArchInit()
{
	/*redirect kprintf to vga console*/
	kprintf_putc = VgaPrintCharacter;
	VgaClearScreen();

	//InitGdb();
}

/*! This function should halt the processor after terminating all the processes
\todo implementation needed
*/
void ArchHalt()
{
	asm("cli;hlt");
}
