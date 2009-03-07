/*!
  \file	tss.c
  \brief	i386 specific task state segment 
*/
#include <ace.h>
#include <kernel/i386/gdt.h>
#include <kernel/i386/processor.h>
#include <kernel/i386/tss.h>

/*! Loads tss register for the current cpu*/
void LoadTss() 
{
    UINT32 tss_address;
	int cpu_number = GetCurrentProcessorId();
    
	tss_address = (UINT32)&processor_i386[cpu_number].tss;
    /* build descriptor */
    gdt[STATIC_GDT_ENTRIES + cpu_number].base_high = tss_address >> 24;
	gdt[STATIC_GDT_ENTRIES + cpu_number].base_mid = (tss_address >> 16) & 0xFF;
	gdt[STATIC_GDT_ENTRIES + cpu_number].base_low = tss_address & 0xFFFF;
	gdt[STATIC_GDT_ENTRIES + cpu_number].segment_limit_high = 0;
	gdt[STATIC_GDT_ENTRIES + cpu_number].segment_limit_low = sizeof(TSS)-1;
	gdt[STATIC_GDT_ENTRIES + cpu_number].descriptor_privilege_level = 0;
	gdt[STATIC_GDT_ENTRIES + cpu_number].default_operation_size = 1;
	gdt[STATIC_GDT_ENTRIES + cpu_number].granularity = 1;
	gdt[STATIC_GDT_ENTRIES + cpu_number].type = 0x9;
	gdt[STATIC_GDT_ENTRIES + cpu_number].present = 1;

	processor_i386[cpu_number].tss.ss0 = KERNEL_DATA_SELECTOR;
	/* set to point beyond the TSS limit */
    processor_i386[cpu_number].tss.iomap = (UINT16) sizeof(TSS)-1;
	
    asm volatile("ltr %%ax": : "a" ((STATIC_GDT_ENTRIES +  cpu_number)<<3));
}
