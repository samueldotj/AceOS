/*!
	\file		arch.c
	\author		Samuel
	\version 	3.0
	\date	
  			Created: 26/09/07 15:21
  			Last modified: Fri Aug 08, 2008  11:34PM
	\brief	contains architecture related interface routines.
*/
#include <string.h>
#include <kernel/debug.h>
#include <kernel/gdb.h>
#include <kernel/multiboot.h>
#include <kernel/parameter.h>
#include <kernel/mm/vm.h>
#include <kernel/mm/pmem.h>
#include <kernel/mm/virtual_page.h>
#include <kernel/i386/vga_text.h>
#include <kernel/i386/gdt.h>
#include <kernel/i386/idt.h>
#include <kernel/i386/exception.h>
#include <kernel/i386/pmem.h>
#include <kernel/i386/cpuid.h>
#include <kernel/processor.h>
#include <kernel/ioapic.h>
#include <kernel/apic.h>
#include <kernel/acpi/acpi.h>

extern void SetupInterruptStubs();
extern UINT32 trampoline_data, trampoline_end;

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
	
	/*execute cpuid and load the data structure for master cpu*/
	LoadCpuIdInfo(&cpuid_info[0]);
	
	/*setup exception handlers and interrupt hanlders*/
	SetupExceptionHandlers();
	SetupInterruptStubs();
	
	if ( mbi->flags & MB_FLAG_CMD )
		sys_kernel_cmd_line = (char *)BOOT_TO_KERNEL_ADDRESS(mbi->cmdline);

	/*enable interrupt only after setting up idt*/    
   // __asm__ __volatile__ ("sti");

}

/*! This function should halt the processor after terminating all the processes
\todo implementation needed
*/
void ArchHalt()
{
	asm("cli;hlt");
}

/*! Flushes the currently executing CPU's cache
	\note use it care fully to avoid performance impact
*/
void FlushCpuCache(BOOLEAN write_back)
{
	if ( write_back )
		asm volatile ("wbinvd");
	else
		asm volatile ("invd");
}
/*! Invalidates all the tlb in the CPU
*/
void InvalidateAllTlb()
{
	/*there is no instruction to clear all TLB in i386, so just rewrite the cr3 
	which will clear all the TLBs	*/
	asm volatile("mov %%cr3, %%eax;\
				  mov %%eax, %%cr3"
				:);
}
/*! Invalidates the tlb for a given va in the CPU
*/
void InvalidateTlb(void * va)
{
	asm volatile("invlpg (%%eax)" : : "a" (va) );
}
/*! First function called from asm stub when a Secondary CPU starts
*/
void SecondaryCPUStart()
{
	kprintf("Secondary CPU is starting...\n");
}
/*! Create a physical page with appropriate real mode code to start a secondary CPU
	\return Physical address of the page
*/
UINT32 CreatePageForSecondaryCPUStart()
{	
	VADDR va;
	VIRTUAL_PAGE_PTR vp;
	int kernel_stack_pages = 2;
	
	vp = AllocateVirtualPages(kernel_stack_pages, VIRTUAL_PAGE_RANGE_TYPE_BELOW_1MB);
	if ( vp == NULL )
		panic("PA not available for starting secondary CPU\n");
	if ( AllocateVirtualMemory(&kernel_map, &va, 0, PAGE_SIZE * kernel_stack_pages, 0, 0) != ERROR_SUCCESS )
		panic("VA not available for starting secondary CPU\n");
	if ( CreatePhysicalMapping(kernel_map.physical_map, va, VP_TO_PHYS(vp), 0) != ERROR_SUCCESS )
		panic("VA to PA mapping failed\n");
		
	/*copy the 16 bit real mode  code*/
	memcpy( (void *)va, (void *)trampoline_data, trampoline_end-trampoline_data);
	
	/*return the physical address*/
	return VP_TO_PHYS(vp);
}

/*!
	\brief	 Get apic information by calling ACPI.

	\param	 void

	\return	 void
*/
void SetupAPIC(void)
{
	ACPI_TABLE_MADT *madt_ptr;
	if ( AcpiGetTable ("APIC", 1, (ACPI_TABLE_HEADER**)(&madt_ptr)) != AE_OK )
		panic("AcpiGetTable() failed\n");
	else
	{
		kprintf("LAPIC Address %p [%s]\n", madt_ptr->Address, madt_ptr->Flags&1?"APIC and Dual 8259 Support":"Only APIC" );
		ACPI_SUBTABLE_HEADER *sub_header, *table_end;
		sub_header = (ACPI_SUBTABLE_HEADER *) ( ((UINT32)madt_ptr) + sizeof(ACPI_TABLE_MADT) );
		table_end = (ACPI_SUBTABLE_HEADER *) ( ((UINT32)madt_ptr) + madt_ptr->Header.Length );

		count_running_processors = 0;
		while ( sub_header < table_end )
		{
			if ( sub_header->Type == ACPI_MADT_TYPE_LOCAL_APIC )
			{
				ACPI_MADT_LOCAL_APIC *p = ( ACPI_MADT_LOCAL_APIC * ) sub_header;
				kprintf("Processor ID %d LAPIC Id = %d [%s]\n", p->ProcessorId, p->Id, ( (p->LapicFlags & 1) ? "Online" : "Offline") );
				processor[count_running_processors].apic_id = p->Id;
				processor[count_running_processors].state = ( (p->LapicFlags & 1) ? (PROCESSOR_STATE_ONLINE) : (PROCESSOR_STATE_OFFLINE) );
				kprintf("processor loaded..\n");
				count_running_processors++;
			}
			else if ( sub_header->Type == ACPI_MADT_TYPE_IO_APIC )
			{
				ACPI_MADT_IO_APIC *p = ( ACPI_MADT_IO_APIC * ) sub_header;
				kprintf("IOAPIC ID %d IOAPIC Physical Address = %p GlobalIRQBase %d\n", p->Id, p->Address, p->GlobalIrqBase);
				ioapic[count_ioapic].ioapic_id = p->Id;
				ioapic[count_ioapic].physical_address = p->Address;
				ioapic[count_ioapic].starting_vector = p->GlobalIrqBase;
				count_ioapic++;
			}
			else
				kprintf("Type = %d\n", sub_header->Type);

			sub_header = (ACPI_SUBTABLE_HEADER *) ( ((UINT32)sub_header) + sub_header->Length );
		}
		kprintf("while done\n");
	}
	InitAPIC();
}
