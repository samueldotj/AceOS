/*!
  \file	tss.c
  \brief	i386 specific task state segment
*/
#include <ace.h>
#include <string.h>
#include <kernel/debug.h>
#include <kernel/mm/vm.h>
#include <kernel/mm/pmem.h>
#include <kernel/mm/virtual_page.h>
#include <kernel/i386/pmem.h>
#include <kernel/i386/gdt.h>
#include <kernel/i386/processor.h>
#include <kernel/i386/tss.h>

/*! task state segment for double fault handling.
	Double faults are usually happens due to stack corruption, if interrupt gate is used to handle double fault then stack corruption will result in a triple fault causing processor reset.
	To avoid this double faults are handled using task gate.
*/
TSS double_fault_tss;

/*! Helper function to fill a TSS and corresponding gdt entry
	\param tss - task state segment to initialize
	\param gdt_index - index in global descriptor table
	\param start_addess - eip will be set with this pointer
	\param kernel_stack - stack for kernel
*/
static void FillTss(TSS_PTR tss, int gdt_index, UINT32 start_address, UINT32 kernel_stack)
{
	UINT32 tss_address = (UINT32)tss;
	
	memset( &gdt[gdt_index], 0, sizeof(struct gdt_entry) );
	gdt[gdt_index].base_high = tss_address >> 24;
	gdt[gdt_index].base_mid = ( tss_address >> 16 ) & 0xFF;
	gdt[gdt_index].base_low = tss_address & 0xFFFF;
	gdt[gdt_index].segment_limit_high = 0;
	gdt[gdt_index].segment_limit_low = sizeof( TSS )-1;
	gdt[gdt_index].descriptor_privilege_level = 0;
	gdt[gdt_index].granularity = 1;
	gdt[gdt_index].type = 0x9;
	gdt[gdt_index].present = 1;

	/* update tss*/
	memset( (void *)tss, 0, sizeof( tss ) );
	tss->cs = KERNEL_CODE_SELECTOR;
	tss->eip = (UINT32)start_address;
	tss->ds = tss->es = tss->fs = tss->gs = KERNEL_DATA_SELECTOR;
	tss->ss = tss->ss0 = tss->ss1 = tss->ss2 = KERNEL_DATA_SELECTOR;
	tss->esp = kernel_stack+PAGE_SIZE;
	tss->eflags = 0x202;
	if ( TranslatePaFromVa( (VADDR)kernel_map.physical_map->page_directory, (VADDR *)&tss->cr3 ) == VA_NOT_EXISTS )
		panic( "kernel page table not present" );

	/* set to point beyond the TSS limit */
	tss->iomap = ( UINT16 ) sizeof( TSS )-1;
}
 
/*! Loads tss register for the current cpu*/
void LoadTss()
{
	int cpu_number = GetCurrentProcessorId();

	FillTss( &processor_i386[cpu_number].tss, STATIC_GDT_ENTRIES+cpu_number, NULL, NULL );
	
	/*load the tss into task register*/
	asm volatile( "ltr %%ax": : "a" ((STATIC_GDT_ENTRIES+cpu_number)<<3) );
}

/*! Setup task gate for double fault handler
	\param 	fault_handler - double fault handler va
*/
void FillTssForDoubleFaultHandler(void * fault_handler)
{
	UINT32 va=0, pa=0;
	
	/*allocate va for double fault handler stack*/
	if ( AllocateVirtualMemory(&kernel_map, &va, 0, PAGE_SIZE, 0, 0, NULL) != ERROR_SUCCESS )
		panic("Unable to allocate memory for double fault handler stack");
	
	/*Create va to pa mapping*/
	((UINT32*)va)[0]=0;
	if ( TranslatePaFromVa( (VADDR)va, &pa ) == VA_NOT_EXISTS )
		panic( "Mapping not exist for double fault hanlder" );
	
	/*Remove the page from pager*/
	LockVirtualPages( PHYS_TO_VP(pa), 1);

	FillTss( &double_fault_tss, DOUBLE_FAULT_GDT_INDEX, (UINT32)fault_handler, va);
}
