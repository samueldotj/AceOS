/*!
	\file	kernel/i386/arch.c
	\brief	contains i386 specific routines.
*/
#include <ace.h>
#include <kernel/arch.h>
#include <kernel/parameter.h>
#include <kernel/pm/thread.h>
#include <kernel/i386/i386.h>

extern void InitInterruptControllers();
extern void Start8254Timer(UINT32 frequency);
extern void SetupInterruptStubs();
extern inline UINT64 rdtsc();

void kprint_vga(void * arg, char ch)
{
	VgaPrintCharacter(ch);
}

/*! Initializes printf and corrects kenrel command line during early boot
	\param mbi - multiboot information passed by multiboot loader(grub)
*/
void InitArchPhase1(MULTIBOOT_INFO_PTR mbi)
{
	/*correct kenrel parameter pointer*/
	if ( mbi->flags & MB_FLAG_CMD )
		sys_kernel_cmd_line = (char *)BOOT_TO_KERNEL_ADDRESS(mbi->cmdline);
		
	/*redirect kprintf to vga console*/
	kprintf_putc = kprint_vga;
	VgaClearScreen();
	
	/*Initialize ktrace*/
	InitKtrace();
}

/*! Initializes i386 specific data structures

	This function is called after InitVm() so it is safe to use kmalloc and other vm functions
	\param mbi - multiboot information
*/
void InitArchPhase2(MULTIBOOT_INFO_PTR mbi)
{
	CPUID_INFO cpuid;
	UINT64 start_time;

	/*execute cpuid and load the data structure*/
	LoadCpuIdInfo( &cpuid );
	memcpy( &processor_i386[cpuid.feature._.apic_id].cpuid, &cpuid, sizeof(CPUID_INFO) );
	master_processor_id = cpuid.feature._.apic_id;
	
	/* create a va to pa mapping for LAPIC address*/
	lapic_base_address = (IA32_APIC_BASE_MSR_PTR) MapPhysicalMemory(&kernel_map, LAPIC_BASE_MSR_START, LAPIC_MEMORY_MAP_SIZE, 0, PROT_READ|PROT_WRITE );
	
	/*intialize master cpu's LACPI*/
	InitLAPIC();
	
	/* Initialize interrupt controllers and start receiving interrupts
		Enabling interrupts are done very early because we need to calculate CPU frequency to program LAPIC timer.
	*/
	InitInterruptControllers();
	
	/* Install interrupt handler for the 8254*/
	InstallInterruptHandler( legacy_irq_redirection_table[LEGACY_DEVICE_IRQ_TIMER], _8254Handler, 0);
	/* Program 8254 timer to interrupt at given frequency per second*/
	Start8254Timer(TIMER_FREQUENCY);
	
	/* Find master CPUs frequency*/
	start_time = rdtsc();
	Delay(10);
	cpu_frequency = (rdtsc() - start_time) * 100;
	kprintf("Primary CPU frequency %d MHz\n", cpu_frequency / (1024*1024) );
	
	/*! There is no way to stop the 8254 so we mask the interrupt*/
	//MaskInterrupt(legacy_irq_redirection_table[LEGACY_DEVICE_IRQ_TIMER]);
	
	/* Install interrupt handler for the LAPIC timer*/
	InstallInterruptHandler( LOCAL_TIMER_VECTOR_NUMBER-32, LapicTimerHandler, 0);
	
	/* Initialize real time clock*/
	InitRtc();
		
	/*setup double fault handler tss and gdt entries*/
	FillTssForDoubleFaultHandler(DoubleFaultHandler);
	
	/* Load TSS so that we can switch to user mode*/
	LoadTss();
}
