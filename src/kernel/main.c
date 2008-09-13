/*!
  \file		main.c
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
#include <kernel/mm/vm.h>
#include <kernel/apic.h>

extern int InitACPI();

/*! first C function which gets control from assembly */
void cmain(unsigned long magic, MULTIBOOT_INFO_PTR mbi)
{
	SYSTEM_TIME boot_time;
	
	/* Initialize architecture depend parts - console and kernel command line*/
	InitArchPhase1(mbi);
	
	/* Initialize kernel parameters and parse boot parameters*/
	InitKernelParameters();
	ParaseBootParameters();

	kprintf( ACE_NAME"%d.%d %s\n", ACE_MAJOR, ACE_MINOR, ACE_BUILD);

	/* Initialize virtual memory manager*/
	InitVm();

	/* Read ACPI tables and enable ACPI mode*/
	if ( InitACPI() != 0 )
		panic("ACPI Initialization failed.\n");

	/* Initialize architecture depend parts*/
	InitArchPhase2(mbi);
	
	/* Start gdb as soon as possible*/
	if ( sys_gdb_port )
		InitGdb();
		
	GetBootTime( &boot_time );
	kprintf("Boot time: %d-%d-%d %d:%d\n", boot_time.day, boot_time.month, boot_time.year, boot_time.hour, boot_time.minute);

	/* Start secondary processors*/
	InitSecondaryProcessors();
	
	kprintf("Kernel initialization complete\n");
}
