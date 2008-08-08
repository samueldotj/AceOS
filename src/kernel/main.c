/*!
  \file		main.c
  \author	DilipSimha N M and Samuel
  \version 	3.0
  \date	
  			Created: Fri Sep 21, 2007  02:26PM
  			Last modified: Fri Aug 08, 2008  11:32PM
  \brief		
*/
#include <version.h>
#include <kernel/arch.h>
#include <kernel/parameter.h>
#include <kernel/debug.h>
#include <kernel/gdb.h>
#include <kernel/multiboot.h>
#include <kernel/mm/pmem.h>
#include <kernel/time.h>
#include <kernel/pit.h>
#include <kernel/mm/vm.h>
#include <kernel/apic.h>

extern int InitACPI();

/*! first C function which gets control from assembly */
void cmain(unsigned long magic, MULTIBOOT_INFO_PTR mbi)
{
	SYSTEM_TIME boot_time;
	
	/*initialize architecture depend parts*/
	ArchInit(mbi);
	
	/*boot error*/
	if ( magic != MULTIBOOT_BOOTLOADER_MAGIC )
	{
		kprintf("Boot loader error (%x). Magic != %x", magic, MULTIBOOT_BOOTLOADER_MAGIC);
		goto fatal_boot_error;
	}
	kprintf( ACE_NAME" Version %d.%d Build%s\n", ACE_MAJOR, ACE_MINOR, ACE_BUILD);

	InitVm();
		
	if ( InitRtc() )
	{
		kprintf("RTC Initialization failed.");
		goto fatal_boot_error;
	}
	GetBootTime( &boot_time );
	kprintf("Boot time: %d-%d-%d %d:%d\n", boot_time.day, boot_time.month, boot_time.year, boot_time.hour, boot_time.minute);

	/* Initialize PIT with the specified frequency */
	if ( InitPit(TIMER_FREQUENCY) )	
	{
		kprintf("PIT Initialization failed");
		goto fatal_boot_error;
	}

	/*initialize kernel parameters and parse boot parameters*/
	InitKernelParameters();
	kprintf("Initialized kernel parameters\n");
	ParaseBootParameters();

	/*start gdb as soon as possible*/
	if ( sys_gdb_port )
		InitGdb();
		
	if ( InitACPI() != 0 )
		panic("ACPI Initialization failed.\n");
	kprintf("ACPI initialized\n");

	SetupAPIC();
	kprintf("APIC initialized\n");
    __asm__ __volatile__ ("sti");

	InitSmp();
	kprintf("SMP initialised\n");
fatal_boot_error:
	while(1);
}
