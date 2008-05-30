/*!
	\file		arch.c
	\author		Samuel
	\version 	3.0
	\date	
  			Created: 26/09/07 15:21
  			Last modified: Tue May 27, 2008  11:05AM
	\brief	contains architecture related interface routines.
*/
#include <kernel/debug.h>
#include <kernel/gdb.h>
#include <kernel/multiboot.h>
#include <kernel/parameter.h>
#include <kernel/mm/vm.h>
#include <kernel/mm/pmem.h>
#include <kernel/i386/vga_text.h>
#include <kernel/i386/gdt.h>
#include <kernel/i386/idt.h>
#include <kernel/i386/exception.h>
#include <kernel/i386/pagetab.h>
#include <kernel/i386/interrupt.h>


/*! This is the startup module for i386 architecture
	This should initialize all the i386 specific data/variables
*/
void ArchInit(MULTIBOOT_INFO_PTR mbi)
{
	/*redirect kprintf to vga console*/
	kprintf_putc = VgaPrintCharacter;
	VgaClearScreen();

	/*load the Global Descriptor Table into GDTR*/
    LoadGdt();
	
	/*load the Interrupt descriptor table into IDTR*/
    LoadIdt();
	/*setup exception handlers and interrupt hanlders*/
	SetupExceptionHandlers();
	SetupInterruptHandlers();
	
	if ( mbi->flags & MB_FLAG_CMD )
		sys_kernel_cmd_line = (char *)BOOT_TO_KERNEL_ADDRESS(mbi->cmdline);
	
	
	/*enable interrupt only after setting up idt*/    
    __asm__ __volatile__ ("sti");

}

/*! This function should halt the processor after terminating all the processes
\todo implementation needed
*/
void ArchHalt()
{
	asm("cli;hlt");
}

