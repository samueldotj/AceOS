/*!
  \file		main.c
  \brief	Kernel startup file
*/
#include <string.h>
#include <version.h>
#include <kernel/arch.h>
#include <kernel/parameter.h>
#include <kernel/printf.h>
#include <kernel/debug.h>
#include <kernel/gdb.h>
#include <kernel/multiboot.h>
#include <kernel/time.h>
#include <kernel/interrupt.h>
#include <kernel/mm/pmem.h>
#include <kernel/mm/kmem.h>
#include <kernel/mm/vm.h>
#include <kernel/pm/task.h>
#include <kernel/pm/thread.h>
#include <kernel/pm/elf.h>
#include <kernel/pm/scheduler.h>
#include <kernel/iom/iom.h>
#include <kernel/system_call_handler.h>
#include <kernel/module.h>
#include <kernel/i386/i386.h>

EXPORT_SYMBOL (AcpiGetName);

/*! ERROR_CODE to string declaration and function defintions*/
AS_STRING_DEC(ERROR_CODE, ENUM_LIST_ERROR_CODE)
AS_STRING_FUNC(ERROR_CODE, ENUM_LIST_ERROR_CODE)

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
	if( kernel_reserve_range.symbol_va_start == NULL )
		kprintf("Warning : Kernel symbols not found - loading of kernel modules not possible\n");

	/* Initialize kernel task and link current thread(boot thread) with it*/
	InitKernelTask();

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
	kprintf("Boot time: %02d-%02d-%02d %02d:%02d\n", boot_time.day, boot_time.month, boot_time.year, boot_time.hour, boot_time.minute);

	/* Initialize architecture independent portion of processor structure*/
	InitProcessors();
	
	/* Initialize scheduler structures*/
	InitScheduler();
	
	/* Initialize architecture depended portion of processor structure and start secondary processors */
	InitSecondaryProcessors();
	
	CompletePhysicalMemoryManagerInit();
	
	/* Start the architecture depended timer for master processor - to enable scheduler */
	StartTimer(SCHEDULER_DEFAULT_QUANTUM, FALSE);
	
	/* Initialize virtual file system */
	InitVfs();
	
	/* Initialize IO manager and start drivers*/
	InitIoManager();
	
	/* Installl system call handler */
	SetupSystemCallHandler();	
	
	//InitGraphicsConsole();
		
	//CreateTask("/boot/app/hello.exe", IMAGE_TYPE_ELF_FILE, TASK_CREATION_FLAG_NONE, NULL, "hello.exe", "TEST=TS");
	
	CreateTask("/boot/app/bash", IMAGE_TYPE_ELF_FILE, TASK_CREATION_FLAG_NONE, NULL,  "bash -i", "ees=33");
	
	kprintf("Kernel initialization complete\n");
	
	ExitThread();
}
