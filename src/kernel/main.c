/*!
  \file		main.c
  \author	DilipSimha N M and Samuel
  \version 	3.0
  \date	
  			Created: Fri Sep 21, 2007  02:26PM
  			Last modified: Fri Oct 12, 2007  04:27PM
  \brief		
*/
#include <version.h>
#include <kernel/arch.h>
#include <kernel/parameter.h>
#include <kernel/debug.h>
#include <kernel/gdb.h>
#include <kernel/multiboot.h>
#include <kernel/time.h>

/*! first C function which gets control from assembly
*/
void cmain(unsigned long magic, multiboot_info_t * mbi)
{
	SYSTEM_TIME boot_time;
	int a=0, b=1;
	
	/*initialize architecture depend parts*/
	ArchInit(mbi);
	
	/*boot error*/
	if ( magic != MULTIBOOT_BOOTLOADER_MAGIC )
	{
		kprintf("Boot loader error (%x). Magic != %x", magic, MULTIBOOT_BOOTLOADER_MAGIC);
		while(1);
	}
	kprintf( ACE_NAME" Version %d.%d Build%s\n", ACE_MAJOR, ACE_MINOR, ACE_BUILD);
	if ( InitRtc() )
	{
		kprintf("RTC Initialization failed.");
		while(1);
	}
	GetBootTime( &boot_time );
	kprintf("Boot time : %d-%d-%d %d:%d\n", boot_time.day, boot_time.month, boot_time.year, boot_time.hour, boot_time.minute);
	
	/*initialize kernel parameters and parse boot parameters*/
	InitKernelParameters();
	ParaseBootParameters();
	
	/*start gdb as soon as possible*/
	if ( sys_gdb_port )
		InitGdb();

	//for now generate exception
	a = b/a;
	
}
