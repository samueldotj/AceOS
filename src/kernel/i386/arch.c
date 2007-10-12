/*!
	\file		arch.c
	\author		Samuel
	\version 	3.0
	\date	
  			Created: 26/09/07 15:21
  			Last modified: Fri Oct 12, 2007  04:26PM
	\brief	contains architecture related interface routines.
*/
#include <kernel/debug.h>
#include <kernel/gdb.h>
#include <kernel/i386/vga_text.h>
#include <kernel/i386/gdt.h>
#include <kernel/i386/idt.h>


/*! This is the startup module for i386 architecture
	This should initialize all the i386 specific data/variables
*/
void ArchInit()
{
	/*redirect kprintf to vga console*/
	kprintf_putc = VgaPrintCharacter;
	VgaClearScreen();

    GdtInstall();
    IdtInstall();
    ExceptionStubInstall();
    InterruptInstall();
    __asm__ __volatile__ ("sti");
	//InitGdb();
}

/*! This function should halt the processor after terminating all the processes
\todo implementation needed
*/
void ArchHalt()
{
	asm("cli;hlt");
}
