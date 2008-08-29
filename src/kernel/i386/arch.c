/*!
	\file	kernel/i386/arch.c
	\brief	contains architecture related interface routines.
*/
#include <ace.h>
#include <string.h>
#include <kernel/io.h>
#include <kernel/debug.h>
#include <kernel/gdb.h>
#include <kernel/multiboot.h>
#include <kernel/parameter.h>
#include <kernel/processor.h>
#include <kernel/ioapic.h>
#include <kernel/apic.h>
#include <kernel/acpi/acpi.h>
#include <kernel/mm/vm.h>
#include <kernel/mm/pmem.h>
#include <kernel/mm/virtual_page.h>
#include <kernel/i386/vga_text.h>
#include <kernel/i386/gdt.h>
#include <kernel/i386/idt.h>
#include <kernel/i386/exception.h>
#include <kernel/i386/pmem.h>
#include <kernel/i386/cpuid.h>
#include <kernel/i386/processor.h>

extern void SetupInterruptStubs();
extern UINT32 trampoline_data, trampoline_end;

PROCESSOR_I386 processor_i386[MAX_PROCESSORS];
UINT16 master_processor_id = 0;

/*! Initializes printf and corrects kenrel command line during early boot
	\param mbi - multiboot information passed by multiboot loader(grub)
*/
void InitArchPhase1(MULTIBOOT_INFO_PTR mbi)
{
	/*correct kenrel parameter pointer*/
	if ( mbi->flags & MB_FLAG_CMD )
		sys_kernel_cmd_line = (char *)BOOT_TO_KERNEL_ADDRESS(mbi->cmdline);
		
	/*redirect kprintf to vga console*/
	kprintf_putc = VgaPrintCharacter;
	VgaClearScreen();
}
/*! Initializes i386 specific data structures

	This function is called after InitVm() so it is safe to use kmalloc and other vm functions
	\param mbi - multiboot information
*/
void InitArchPhase2(MULTIBOOT_INFO_PTR mbi)
{
	CPUID_INFO cpuid;

	/*execute cpuid and load the data structure*/
	LoadCpuIdInfo( &cpuid );
	memcpy( &processor_i386[cpuid.feature._.apic_id].cpuid, &cpuid, sizeof(CPUID_INFO) );
	master_processor_id = cpuid.feature._.apic_id;
	
	/* create a va to pa mapping for LAPIC address*/
	lapic_base_address = (IA32_APIC_BASE_MSR_PTR) MapPhysicalMemory(&kernel_map, LAPIC_BASE_MSR_START, LAPIC_MEMORY_MAP_SIZE );
	
	/*intialize master cpu's LACPI*/
	InitLAPIC();
}

/*! returns the current processor's LAPIC id
	\todo - implement this without CPUID to avoid performance issues
*/
UINT16 GetCurrentProcessorId()
{
	CPUID_INFO cpuid_info;
	LoadCpuIdInfo( &cpuid_info );
	
	return cpuid_info.feature._.apic_id;
}

/*! First function called from asm stub when a Secondary CPU starts
*/
void SecondaryCpuStart()
{
	CPUID_INFO cpuid;
	
	/*inform master CPU that I started*/
	count_running_processors++;
	
	/*execute cpuid and load the data structure*/
	LoadCpuIdInfo( &cpuid );
	memcpy( &processor_i386[cpuid.feature._.apic_id].cpuid, &cpuid, sizeof(CPUID_INFO) );
	
	//kprintf("Secondary CPU %d is started\n", cpuid.feature._.apic_id);
	
}
/*! Create a physical page with appropriate real mode code to start a secondary CPU
	\return Physical address of the page
*/
static UINT32 CreatePageForSecondaryCPUStart()
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
	memcpy( (void *)va, (void *)&trampoline_data, ((UINT32)&trampoline_end)-((UINT32)&trampoline_data));
	
	/*return the physical address*/
	return VP_TO_PHYS(vp);
}

/*! Initializes the Secondary processors and IOAPIC

	1) Read the ACPI tables to processor count and their ids
	2) While reading the table start processors if LAPIC ids are found
		This is done here beacause ACPI spec says, processor initialization should be done in the same order as they appear in the APIC table
	3) If IOAPIC id is found during the table read store it.
*/
void InitSmp()
{
	ACPI_TABLE_MADT *madt_ptr;
	ACPI_SUBTABLE_HEADER *sub_header, *table_end;
	ACPI_STATUS result;
	int i, disable_8259=0;
	
	/* disable all interrupts*/
	asm volatile("cli");
	
	/*! Initialize the PIC*/
	InitPIC(PIC_STARTING_VECTOR_NUMBER);
	/*initialize the legacy IRQ to interrupt mapping table*/
	for(i=0;i<16;i++)
		legacy_irq_redirection_table[i]=i;
	
	/*! try to read APIC table*/
	result = AcpiGetTable ("APIC", 1, (ACPI_TABLE_HEADER**)(&madt_ptr));
	if ( result == AE_OK )
	{
		kprintf("LAPIC Address %p [%s]\n", madt_ptr->Address, madt_ptr->Flags&1 ? "APIC and Dual 8259 Support" : "Only APIC" );
		/* if the machine has both 8259 and IOAPIC support disable the 8259*/
		if ( madt_ptr->Flags & 1 )
			disable_8259 = 1;
	
		sub_header = (ACPI_SUBTABLE_HEADER *) ( ((UINT32)madt_ptr) + sizeof(ACPI_TABLE_MADT) );
		table_end = (ACPI_SUBTABLE_HEADER *) ( ((UINT32)madt_ptr) + madt_ptr->Header.Length );

		/*only master is running now*/
		count_running_processors = 1;
		count_ioapic = 0;
		while ( sub_header < table_end )
		{
			if ( sub_header->Type == ACPI_MADT_TYPE_LOCAL_APIC )
			{
				ACPI_MADT_LOCAL_APIC *p = ( ACPI_MADT_LOCAL_APIC * ) sub_header;
				if ( (p->LapicFlags & 1)  && (p->Id < MAX_PROCESSORS) )
				{
					processor[p->ProcessorId].state = ( (p->LapicFlags & 1) ? (PROCESSOR_STATE_ONLINE) : (PROCESSOR_STATE_OFFLINE) );
					processor_i386[p->ProcessorId].apic_id = p->Id;
					/*master processor is already running, so dont try to start it again*/
					if ( processor_i386[p->ProcessorId].apic_id != master_processor_id )
					{
						UINT32 pa = CreatePageForSecondaryCPUStart();
						kprintf("Starting secondary processor (ID %d LAPIC %d) \n", p->ProcessorId, p->Id );
						StartProcessor( processor_i386[p->ProcessorId].apic_id, pa );
					}
				}
				else
				{
					kprintf("CPU %d (APIC id %d state %d) cant added to the processor array\n", p->ProcessorId, p->Id, p->LapicFlags);
				}
			}
			else if ( sub_header->Type == ACPI_MADT_TYPE_IO_APIC )
			{
				ACPI_MADT_IO_APIC *p = ( ACPI_MADT_IO_APIC * ) sub_header;
				kprintf("IOAPIC ID %d IOAPIC Physical Address = %p GlobalIRQBase %d\n", p->Id, p->Address, p->GlobalIrqBase);
				
				/*!initialize IOAPIC structures*/
				ioapic[count_ioapic].ioapic_id = p->Id;
				ioapic[count_ioapic].starting_vector = p->GlobalIrqBase;
				ioapic[count_ioapic].base_physical_address = (void *) p->Address;
				ioapic[count_ioapic].base_virtual_address = (void *) MapPhysicalMemory(&kernel_map, p->Address, PAGE_SIZE);
				
				/*! Mask the interrupts*/
				MaskIoApic(ioapic[count_ioapic].base_virtual_address);
							
				count_ioapic++;
			}
			else if ( sub_header->Type == ACPI_MADT_TYPE_INTERRUPT_OVERRIDE )
			{
				ACPI_MADT_INTERRUPT_OVERRIDE * override = (ACPI_MADT_INTERRUPT_OVERRIDE *)sub_header;
				legacy_irq_redirection_table[override->Bus] = override->GlobalIrq;
			}
			else if ( sub_header->Type == ACPI_MADT_TYPE_INTERRUPT_SOURCE )
			{
				kprintf("ACPI INTERRUPT INTERRUPT SOURCE STRUCTURE PRESENT BUT KERNEL YET TO IMPLEMENT IT\n");
			}
			else if ( sub_header->Type == ACPI_MADT_TYPE_NMI_SOURCE )
			{
				kprintf("ACPI INTERRUPT NMI SOURCE STRUCTURE PRESENT BUT KERNEL YET TO IMPLEMENT IT\n");
			}
			else
			{
				kprintf("ACPI MADT Structure type %d is not yet implemented\n", sub_header->Type);
			}

			sub_header = (ACPI_SUBTABLE_HEADER *) ( ((UINT32)sub_header) + sub_header->Length );
		}
	}
	else
	{
		#ifdef CONFIG_SMP
			kprintf("AcpiGetTable() failed Code=%d, assuming uniprocessor and using 8259 PIC\n", result);
		#endif
		disable_8259 = 0;
	}
	
	/*! disable PIC if we dont need it*/
	if ( disable_8259 )
		MaskPic();
	
	/*! enable interrupts*/
	asm volatile("sti");
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
