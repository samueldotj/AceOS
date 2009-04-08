/*!
  \file		exception.c
  \brief	
*/

#include <ace.h>
#include <kernel/mm/vm.h>
#include <kernel/i386/idt.h>
#include <kernel/i386/exception.h>
#include <kernel/debug.h>
#include <kernel/arch.h>
#include <kernel/error.h>
#include <kernel/pm/elf.h>

/*	This is a simple string array. It contains the message that corresponds to each and every exception. 
*	We get the correct message by accessing like:  exception_message[interrupt_number] 
*/
char *exception_messages[] =
{
	"Division By Zero",
	"Debug",
	"Non Maskable Interrupt",
	"Breakpoint",
	"Into Detected Overflow",
	"Out of Bounds",
	"Invalid Opcode",			
	"No Coprocessor",			
	"Double Fault",
	"Coprocessor Segment Overrun",
	"Bad TSS",
	"Segment Not Present",
	"Stack Fault",
	"General Protection Fault",
	"Page Fault",
	"Unknown Interrupt",
	"Coprocessor Fault",
	"Alignment Check",
	"Machine Check",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved"
};

#define REG_FORMAT	"0x%08x\t"
#define SEG_FORMAT	"0x%02x"

#define PRINT_REGS(reg)\
	{\
	int offset;\
	char * func;\
	kprintf("eax="REG_FORMAT"ebx="REG_FORMAT"ecx="REG_FORMAT"edx="REG_FORMAT"\n", reg->eax, reg->ebx, reg->ecx, reg->edx);\
	kprintf("esi="REG_FORMAT"edi="REG_FORMAT"ebp="REG_FORMAT"esp="REG_FORMAT"\n", reg->esi, reg->edi, reg->ebp, reg->esp);\
	kprintf("cs:eip="SEG_FORMAT":"REG_FORMAT"[user stack ss:esp="SEG_FORMAT":"REG_FORMAT"]\n", reg->cs, reg->eip, reg->ss, reg->useresp);\
	kprintf("ds="SEG_FORMAT" es="SEG_FORMAT" gs="SEG_FORMAT" fs="SEG_FORMAT" eflags=0x%X err=0x%X\n", reg->ds, reg->es, reg->gs, reg->fs, reg->eflags, reg->error_code);\
	kprintf("cr0="REG_FORMAT" cr1="REG_FORMAT" cr2="REG_FORMAT" cr3="REG_FORMAT"\n", reg->cr0, reg->cr1, reg->cr2, reg->cr3);\
	func = FindKernelSymbolByAddress(reg->eip, &offset);\
	kprintf("Fault at %s+%p\n", func, offset);\
	}

/*! All of our Exception handling Interrupt Service Routines will point to this function. 
*  This will tell us what exception has happened. 
*/
void ExceptionHandler(REGS_PTR reg)
{
	kprintf("######### Unhandled Exception [%s] - No [%d]########\n", exception_messages[reg->int_no], reg->int_no);
	/*now print some useful info from regs structure for debugging*/
	PRINT_REGS( reg );
	kprintf("System Halted!\n");
	ArchHalt();
}

/*! i386 specific page fault handler*/
void PageFaultHandler(REGS_PTR reg)
{
	PF_ERROR_CODE err;
	static int inside_page_fault;
	err.all = reg->error_code;
	if ( inside_page_fault )
	{
		PRINT_REGS(reg);
		panic("Inside page fault");
	}
		
	inside_page_fault++;
	if ( MemoryFaultHandler( reg->cr2, err.user, err.write ) != ERROR_SUCCESS )
	{
		if ( err.rsvd ) 
			kprintf( "Page fault - RESERVED BIT SET\n");
		else
			kprintf("Page fault(code %d): %s page %s attempt : %s\n",  
				reg->error_code,
				err.user ? "User" : "Supervisor" ,
				err.write ? "write" : "read",
				err.present ? "protection failure" : "page not present");
	
		PRINT_REGS(reg);
		panic("Page fault not handled.");
		ArchHalt();
	}
	inside_page_fault--;
}

/*! General Protection fault handler
	\todo - Add code to just terminate the current task and not to panic
*/
void GeneralProtectionFaultHandler(REGS_PTR reg)
{
	GPF_ERROR_CODE err;
	err.all = reg->error_code;
	kprintf("General Protection Fault:%s %s descriptor table (index - %d)\n",
		err.ext ? "External event" : "", 
		err.idt ? "Interrupt" : err.ti ? "Local" : "Global",
		err.segment_selector_index);
	
	PRINT_REGS(reg);
	ArchHalt();
}

/*! Double fault handler
	Runs as different task; just to make sure we panic correctly
*/
void DoubleFaultHandler()
{
	panic("Double fault: System halted!");
}
