/*!
  \file		main.c
  \brief	Kernel startup file
*/
#include <string.h>
#include <version.h>
#include <kernel/arch.h>
#include <kernel/parameter.h>
#include <kernel/debug.h>
#include <kernel/gdb.h>
#include <kernel/multiboot.h>
#include <kernel/module.h>
#include <kernel/mm/pmem.h>
#include <kernel/mm/kmem.h>
#include <kernel/time.h>
#include <kernel/mm/vm.h>
#include <kernel/apic.h>
#include <kernel/pm/task.h>

extern int InitACPI();

THREAD_CONTAINER_PTR kthread1, kthread2;
void TestThread1();
void TestThread2();

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

	/* Initialize architecture independent portion of processor structure*/
	InitProcessors();
	
	/* Initialize scheduler structures*/
	InitScheduler();
	
	/* Initialize architecture depended portion of processor structure and start secondary processors */
	InitSecondaryProcessors();
		
	kthread1 = CreateThread(TestThread1, 0);
	kthread2 = CreateThread(TestThread2, 0);
	
	/* Start the architecture depended timer for master processor - to enable scheduler */
	StartTimer(SCHEDULER_DEFAULT_QUANTUM, TRUE);

	kprintf("Kernel initialization complete\n");
}

void TestThread1()
{
	int i=0;
	kprintf("TestThread1 is started\n");
	while(1)
	{
		kprintf("####(%d) ", i);
		i++;
	}
}
void TestThread2()
{
	int i=0;
	kprintf("TestThread2 is started\n");
	while(1)
	{
		kprintf("***-[%d] ", i);
		i++;
	}
}
