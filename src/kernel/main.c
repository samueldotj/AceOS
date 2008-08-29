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
#include <kernel/pit.h>
#include <kernel/mm/vm.h>
#include <kernel/apic.h>

extern int InitACPI();

/*! first C function which gets control from assembly */
void cmain(unsigned long magic, MULTIBOOT_INFO_PTR mbi)
{
	SYSTEM_TIME boot_time;
	
	/*initialize architecture depend parts - console and kernel command line*/
	InitArchPhase1(mbi);
	/*initialize kernel parameters and parse boot parameters*/
	InitKernelParameters();
	ParaseBootParameters();

	kprintf( ACE_NAME" Version %d.%d Build%s\n", ACE_MAJOR, ACE_MINOR, ACE_BUILD);

	/*initialize virtual memory manager*/
	InitVm();
	
	/*initialize architecture depend parts*/
	InitArchPhase2(mbi);
		
	/*start gdb as soon as possible*/
	if ( sys_gdb_port )
		InitGdb();
		
	if ( InitACPI() != 0 )
		panic("ACPI Initialization failed.\n");
		
	if ( InitRtc() )
		panic("RTC Initialization failed.");
		
	GetBootTime( &boot_time );
	kprintf("Boot time: %d-%d-%d %d:%d\n", boot_time.day, boot_time.month, boot_time.year, boot_time.hour, boot_time.minute);

	/* Initialize PIT with the specified frequency */
	if ( InitPit(TIMER_FREQUENCY) )	
		panic("PIT Initialization failed");
	
	InitSmp();
	
	kprintf("Kernel initialization complete\n");
}
