/*!
  \file		exception.c
  \brief	
*/

#include <ace.h>
#include <kernel/mm/vm.h>
#include <kernel/debug.h>
#include <kernel/arch.h>
#include <kernel/error.h>
#include <kernel/pm/elf.h>
#include <kernel/pm/thread.h>
#include <kernel/i386/i386.h>
#include <kernel/pm/pm_types.h>

/*max depth of page faults inside page fault*/
#define MAX_SERIAL_PAGE_FAULTS	2

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

static inline void PrintRegisterValues(REGS_PTR reg)
{
	kprintf("eax="REG_FORMAT"ebx="REG_FORMAT"ecx="REG_FORMAT"edx="REG_FORMAT"\n", reg->eax, reg->ebx, reg->ecx, reg->edx);
	kprintf("esi="REG_FORMAT"edi="REG_FORMAT"ebp="REG_FORMAT"esp="REG_FORMAT"\n", reg->esi, reg->edi, reg->ebp, reg->esp);
	kprintf("cr0="REG_FORMAT"cr1="REG_FORMAT"cr2="REG_FORMAT"cr3="REG_FORMAT"\n", reg->cr0, reg->cr1, reg->cr2, reg->cr3);
	kprintf("ds="SEG_FORMAT" es="SEG_FORMAT" gs="SEG_FORMAT" fs="SEG_FORMAT" eflags=0x%X err=0x%X\n", reg->ds, reg->es, reg->gs, reg->fs, reg->eflags, reg->error_code);
	kprintf("cs:eip="SEG_FORMAT":"REG_FORMAT" user ss:esp="SEG_FORMAT":"REG_FORMAT"\n", reg->cs, reg->eip, reg->ss, reg->useresp);
	if( reg->eflags & EFLAG_VM86 )
	{
		REGS_V86_PTR v86_reg = (REGS_V86_PTR)reg;
		/*if v86 thread print v86 specific segment registers also*/
		kprintf("V86 ds="SEG_FORMAT" es="SEG_FORMAT" gs="SEG_FORMAT" fs="SEG_FORMAT"\n", v86_reg->v86.ds, v86_reg->v86.es, v86_reg->v86.gs, v86_reg->v86.fs );
	}
	else
	{
		int offset;
		char * func;
		func = FindKernelSymbolByAddress(reg->eip, &offset);
		kprintf("Fault at %s+%p\n", func, offset);
	}
}

/*! All of our Exception handling Interrupt Service Routines will point to this function. 
*  This will tell us what exception has happened. 
*/
void ExceptionHandler(REGS_PTR reg)
{
	kprintf("######### Unhandled Exception [%s] - No [%d]########\n", exception_messages[reg->int_no], reg->int_no);
	/*now print some useful info from regs structure for debugging*/
	PrintRegisterValues( reg );
	kprintf("System Halted!\n");
	ArchShutdown();
}

/*! i386 specific page fault handler*/
void PageFaultHandler(REGS_PTR reg)
{
	VIRTUAL_MAP_PTR virtual_map;
	VADDR va;
	PF_ERROR_CODE err;
	static int inside_page_fault;
	
	va = reg->cr2;
	err.all = reg->error_code;
	
	virtual_map = GetCurrentVirtualMap();
	
	/*page fault can happen while handling a page fault but not more than MAX_SERIAL_PAGE_FAULTS*/
	if ( inside_page_fault > MAX_SERIAL_PAGE_FAULTS )
	{
		PrintRegisterValues(reg);
		panic("Inside page fault");
	}
	inside_page_fault++;
	
	/*for v86 tasks identity map first 1mb*/
	if( reg->eflags & EFLAG_VM86 )
	{
		if( va < (1024*1024) && GetVmDescriptor(virtual_map, va, PAGE_SIZE) == NULL  )
		{
			VADDR new_va;
			va = PAGE_ALIGN(va);
			new_va = MapPhysicalMemory( GetCurrentVirtualMap(), va, PAGE_SIZE, va, PROT_READ | PROT_WRITE);
			if ( new_va != va )
			{
				FreeVirtualMemory(GetCurrentVirtualMap(), va, PAGE_SIZE, 0);
				CreatePhysicalMapping(GetCurrentVirtualMap()->physical_map, va, va, PROT_READ | PROT_WRITE);
			}
			goto done;
		}
	}
	/*let the vm to handle the fault*/
	if ( MemoryFaultHandler( va, err.user, err.write ) != ERROR_SUCCESS )
	{
		if ( err.rsvd ) 
			kprintf( "Page fault - RESERVED BIT SET\n");
		else
			kprintf("Page fault(code %d): %s page %s attempt : %s\n",  
				reg->error_code,
				err.user ? "User" : "Supervisor" ,
				err.write ? "write" : "read",
				err.present ? "protection failure" : "page not present");
	
		PrintRegisterValues(reg);
		panic("Page fault not handled.");
		ArchShutdown();
	}

done:
	inside_page_fault--;
}

/*! General Protection fault handler
*/
void GeneralProtectionFaultHandler(REGS_PTR reg)
{
	GPF_ERROR_CODE err;
	err.all = reg->error_code;
	if( reg->eflags & EFLAG_VM86 )
	{
		if ( GeneralProtectionFaultHandlerForV86Mode( (REGS_V86_PTR) reg) == TRUE )
		{
			return;
		}
	}
	
	/*! \todo - Add code to just terminate the current task and not to panic*/
	kprintf("General Protection Fault:%s %s descriptor table (index - %d)\n",
		err.ext ? "External event" : "", 
		err.idt ? "Interrupt" : err.ti ? "Local" : "Global",
		err.segment_selector_index);
	
	PrintRegisterValues(reg);
	ArchShutdown();	
}

/*! Double fault handler
	Runs as different task; just to make sure we panic correctly
*/
void DoubleFaultHandler()
{
	panic("Double fault: System halted!");
}

#define VALID_FLAGS         0xDFF
/*! General Protection Handler for virtual 8086 mode
	Credit goes to Mobius OS
*/
int GeneralProtectionFaultHandlerForV86Mode(REGS_V86_PTR v86_reg)
{
	BYTE *ip;
    UINT16 *stack, *ivt;
    UINT32 *stack32, pa;
    int is_operand32, is_address32;
	THREAD_PTR thread;
	THREAD_I386_PTR i386_thread;
	REGS_PTR reg;
	
	thread = GetCurrentThread();
	assert( thread != NULL );
	i386_thread = (THREAD_I386_PTR)thread->arch_data;
	assert( i386_thread != NULL );
	assert( i386_thread->is_v86==1 );
	
	reg = &v86_reg->reg;

	ip = FP_TO_LINEAR(reg->cs, reg->eip);
    ivt = (UINT16 *) 0;
    stack = (UINT16 *) FP_TO_LINEAR(reg->ss, reg->useresp);
    stack32 = (UINT32 *) stack;
    is_operand32 = is_address32 = FALSE;
	
	/*create translations for all things that we might touch*/
	if( TranslatePaFromVa((UINT32)ivt, &pa) == VA_NOT_EXISTS )
		CreatePhysicalMapping( GetCurrentVirtualMap()->physical_map, (UINT32)ivt, (UINT32)ivt, PROT_READ | PROT_WRITE);
	if( TranslatePaFromVa((UINT32)stack, &pa) == VA_NOT_EXISTS )
		CreatePhysicalMapping( GetCurrentVirtualMap()->physical_map, (UINT32)stack, (UINT32)stack, PROT_READ | PROT_WRITE);
	if( TranslatePaFromVa((UINT32)stack32, &pa) == VA_NOT_EXISTS )
		CreatePhysicalMapping( GetCurrentVirtualMap()->physical_map, (UINT32)stack32, (UINT32)stack32, PROT_READ | PROT_WRITE);
	
    while (TRUE)
    {
        switch (ip[0])
        {
			case 0x66:            /* O32 */
				is_operand32 = TRUE;
				ip++;
				reg->eip = (UINT16 ) (reg->eip + 1);
				break;

			case 0x67:            /* A32 */
				is_address32 = TRUE;
				ip++;
				reg->eip = (UINT16 ) (reg->eip + 1);
				break;
				
			case 0x9c:            /* PUSHF */
				if (is_operand32)
				{
					reg->useresp = ((reg->useresp & 0xffff) - 4) & 0xffff;
					stack32--;
					stack32[0] = reg->eflags & VALID_FLAGS;

					if (i386_thread->eflag_if)
						stack32[0] |= EFLAG_IF;
					else
						stack32[0] &= ~EFLAG_IF;
				}
				else
				{
					reg->useresp = ((reg->useresp & 0xffff) - 2) & 0xffff;
					stack--;
					stack[0] = (UINT16 ) reg->eflags;

					if (i386_thread->eflag_if)
						stack[0] |= EFLAG_IF;
					else
						stack[0] &= ~EFLAG_IF;
				}

				reg->eip = (UINT16 ) (reg->eip + 1);
				return TRUE;

			case 0x9d:            /* POPF */
				if (is_operand32)
				{
					reg->eflags =  EFLAG_VM86 | (stack32[0] & VALID_FLAGS);
					i386_thread->eflag_if = (stack32[0] & EFLAG_IF) != 0;
					reg->useresp = ((reg->useresp & 0xffff) + 4) & 0xffff;
				}
				else
				{
					reg->eflags = EFLAG_VM86 | stack[0];
					i386_thread->eflag_if = (stack[0] & EFLAG_IF) != 0;
					reg->useresp = ((reg->useresp & 0xffff) + 2) & 0xffff;
				}
				
				reg->eip = (UINT16 ) (reg->eip + 1);
				return TRUE;

			case 0xcd:            /* INT n */
				switch (ip[1])
				{
					case 0x13:
						i386_thread->interrupt_handler(reg);
						reg->eip = (UINT16 ) (reg->eip + 2);
						return TRUE;
						
					default:
						stack -= 3;
						reg->useresp = ((reg->useresp & 0xffff) - 6) & 0xffff;
						stack[0] = (UINT16 ) (reg->eip + 2);
						stack[1] = reg->cs;
						stack[2] = (UINT16 ) reg->eflags;
						
						if (i386_thread->eflag_if)
							stack[2] |= EFLAG_IF;
						else
							stack[2] &= ~EFLAG_IF;

						reg->cs = ivt[ip[1] * 2 + 1];
						reg->eip = ivt[ip[1] * 2];
						return TRUE;
				}
				break;

			case 0xcf:            /* IRET */
				reg->eip = stack[0];
				reg->cs = stack[1];
				reg->eflags = EFLAG_VIF |  EFLAG_IF | EFLAG_VM86 | stack[2];
				i386_thread->eflag_if = (stack[2] & EFLAG_IF) != 0;

				reg->useresp = ((reg->useresp & 0xffff) + 6) & 0xffff;
				return TRUE;

			case 0xfa:            /* CLI */
				i386_thread->eflag_if = FALSE;
				reg->eip = (UINT16 ) (reg->eip + 1);
				return TRUE;

			case 0xfb:            /* STI */
				i386_thread->eflag_if = TRUE;
				reg->eip = (UINT16 ) (reg->eip + 1);
				return TRUE;

			default:
				kprintf("virtual86 - unhandled opcode 0x%x\n", ip[0]);
				reg->eip = (UINT16 ) (reg->eip + 1);
				return TRUE;
			}
    }
    return FALSE;
}
