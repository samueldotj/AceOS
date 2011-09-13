/*!
	\file	kernel/i386/processor.c
	\brief	x86 processor specific routines
*/
#include <ace.h>
#include <kernel/arch.h>
#include <kernel/parameter.h>
#include <kernel/pm/thread.h>
#include <kernel/i386/i386.h>

extern UINT32 trampoline_data, trampoline_end;

/*! i386 processor information*/
PROCESSOR_I386 processor_i386[MAX_PROCESSORS];
/*! master processor id*/
UINT16 master_processor_id = 0;

static UINT32 CreateSecondaryCPUStartThread();


/*! CPU frequency - Ace requires all the processor to run at the same frequency
	This variable should populated during init.
*/
UINT32 cpu_frequency = 0;

/*! ReaD Time Stamp Counter
 * Reads the current processor time stamp using the rdtsc instruction.
 * cpuid instruction is used to serialize the read operation. 
 * \return current processor time stamap
 */
inline UINT64 rdtsc() 
{
	UINT32 lo, hi;
	/*
	* cpuid will serialize the following rdtsc with respect to all other
	* instructions the processor may be handling.
	*/
	__asm__ __volatile__ (
		"xorl %%eax, %%eax\n"
		"cpuid\n"
		"rdtsc\n"
		: "=a" (lo), "=d" (hi)
		:
		: "%ebx", "%ecx");
	return (UINT64)hi << 32 | lo;
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

/*! Initializes the Secondary processors and IOAPIC

	1) Read the ACPI tables to processor count and their ids
	2) While reading the table start processors if LAPIC ids are found
		This is done here beacause ACPI spec says, processor initialization should be done in the same order as they appear in the APIC table
	3) If IOAPIC id is found during the table read store it.
*/
void InitSecondaryProcessors()
{
	ACPI_TABLE_MADT *madt_ptr;
	ACPI_SUBTABLE_HEADER *sub_header, *table_end;
	ACPI_STATUS result;
	
	/*only master is running now*/
	count_running_processors = 1;
		
	/*! try to read APIC table*/
	result = AcpiGetTable ("APIC", 1, (ACPI_TABLE_HEADER**)(&madt_ptr));
	if ( result == AE_OK )
	{
		sub_header = (ACPI_SUBTABLE_HEADER *) ( ((UINT32)madt_ptr) + sizeof(ACPI_TABLE_MADT) );
		table_end = (ACPI_SUBTABLE_HEADER *) ( ((UINT32)madt_ptr) + madt_ptr->Header.Length );

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
						UINT32 pa = CreateSecondaryCPUStartThread();
						kprintf("Starting secondary processor (ID %d LAPIC %d) \n", p->ProcessorId, p->Id );
						StartProcessor( processor_i386[p->ProcessorId].apic_id, pa );
					}
				}
				else
				{
					kprintf("CPU %d (APIC id %d state %d) cant added to the processor array\n", p->ProcessorId, p->Id, p->LapicFlags);
				}
			}
			sub_header = (ACPI_SUBTABLE_HEADER *) ( ((UINT32)sub_header) + sub_header->Length );
		}
	}
}

/*! First function called from asm stub when a Secondary CPU starts
*/
void SecondaryCpuStart()
{
	CPUID_INFO cpuid;
	int processor_id;
	
	/* inform master CPU that I started*/
	count_running_processors++;
	
	/* execute cpuid and load the data structure*/
	LoadCpuIdInfo( &cpuid );
	processor_id = cpuid.feature._.apic_id;
	memcpy( &processor_i386[processor_id].cpuid, &cpuid, sizeof(CPUID_INFO) );
	
	/* initalize the boot thread*/
	InitBootThread( processor_id );
	
	/* initialize the lapic*/
	InitLAPIC();

	/*! enable interrupts*/
	asm volatile("sti");
	
	/* Install interrupt handler for the LAPIC timer*/
	InstallInterruptHandler( LOCAL_TIMER_VECTOR_NUMBER-32, LapicTimerHandler, 0);

	/* Load TSS so that we can switch to user mode*/
	LoadTss();

	/* Start the architecture depended timer for secondary processor - to enable scheduler */
	StartTimer(SCHEDULER_DEFAULT_QUANTUM, TRUE);
	
	kprintf("Secondary CPU %d is started\n", processor_id);
	
	ExitThread();
}

/*! Create a thread with appropriate real mode code to start a secondary CPU
	\return Physical address of the stack of the thread. This stack contains real mode code to start.
*/
static UINT32 CreateSecondaryCPUStartThread()
{	
	VADDR va;
	VIRTUAL_PAGE_PTR vp;
	int i, stack_page_index, size = PAGE_ALIGN_UP( sizeof(THREAD_CONTAINER) );
	
	/*allocate contiguous physical pages*/
	if ( (vp = AllocateVirtualPages( size/PAGE_SIZE , VIRTUAL_PAGE_RANGE_TYPE_BELOW_1MB)) == NULL )
		panic("PA not available for starting secondary CPU\n");
		
	/*reserve kernel virtual address*/
	if ( AllocateVirtualMemory(&kernel_map, &va, 0, size, 0, VM_UNIT_FLAG_PRIVATE, NULL) != ERROR_SUCCESS )
		panic("VA not available for starting secondary CPU\n");
	
	/*create va to pa mapping*/
	for (i=0;i<size/PAGE_SIZE;i++)
	{
		if ( CreatePhysicalMapping(kernel_map.physical_map, va+(i*PAGE_SIZE), VP_TO_PHYS(vp+i), 0) != ERROR_SUCCESS )
			panic("VA to PA mapping failed\n");
	}
	
	/*copy the 16 bit real mode  code to the kernel stack page*/
	memcpy( &((THREAD_CONTAINER_PTR)va)->kernel_stack, (void *)&trampoline_data, ((UINT32)&trampoline_end)-((UINT32)&trampoline_data));
	
	stack_page_index =  OFFSET_OF_MEMBER(THREAD_CONTAINER, kernel_stack) / PAGE_SIZE;
	/*return the physical address of the stack*/
	return VP_TO_PHYS(vp+stack_page_index);
}

/*! Halt the processor until external interrupt comes*/
void ArchHalt()
{
	asm("hlt");
}

/*! Take the cpu to offline
	\todo implementation needed using acpi
*/
void ArchShutdown()
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
