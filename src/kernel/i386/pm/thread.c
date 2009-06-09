/*!
  \file	kernel/i386/pm/thread.c
  \brief	i386 specific thread management routines
*/

#include <ace.h>
#include <kernel/debug.h>
#include <kernel/arch.h>
#include <kernel/pm/pm_types.h>
#include <kernel/pm/task.h>
#include <kernel/pm/thread.h>
#include <kernel/i386/pmem.h>
#include <kernel/i386/exception.h>
#include <kernel/i386/gdt.h>
#include <kernel/i386/ioapic.h>

#define EFLAG_VALUE			0x202

/*gets the current kernel stack position*/
#define GET_KERNEL_STACK(kstack) 		asm volatile("movl %%esp, %0":"=m"(kstack))

extern void ReturnFromInterruptContext();

/*! returns current kernel stack pointer*/
inline BYTE * GetKernelStackPointer()
{
	BYTE * kstack;
	GET_KERNEL_STACK( kstack );
	return (BYTE *)PAGE_ALIGN(kstack);
}

/*! Fills the given thread container with default register values
	\param thread_container - thread container
	\param start_address - starting function pointer of the thread
*/
void FillThreadContext(THREAD_CONTAINER_PTR thread_container, void * start_address, BYTE is_kernel_thread, VADDR user_stack)
{
	UINT32 * stack_frame;
	REGS_PTR regs;
	UINT32 cr3;
	
	assert( thread_container != NULL );
	
	/*build last stackframe and point it to ExitThread*/
	stack_frame = (UINT32 *) (thread_container->kernel_stack + sizeof(thread_container->kernel_stack));
	stack_frame[-2] = (UINT32)ExitThread;
	
	/*build exception frame*/
	regs = (REGS_PTR)( ((BYTE*)stack_frame) - sizeof(REGS) );
	if ( is_kernel_thread )
	{
		regs->cs = KERNEL_CODE_SELECTOR;
		regs->ds = regs->es = regs->gs = regs->fs = regs->ss = KERNEL_DATA_SELECTOR;
	}
	else
	{
		regs->cs = USER_CODE_SELECTOR | USER_PRIVILEGE_LEVEL;
		regs->ds = regs->es = regs->gs = regs->fs = regs->ss = USER_DATA_SELECTOR | USER_PRIVILEGE_LEVEL;
		regs->useresp = user_stack + USER_STACK_SIZE; /*stack grows from top to bottom*/
	}
		
	regs->eip = (UINT32)start_address;
	regs->eflags = EFLAG_VALUE;
	
	/*get the physical address of the page directory*/
	if( TranslatePaFromVa((VADDR)thread_container->thread.task->virtual_map->physical_map->page_directory, &cr3) == VA_NOT_EXISTS )
		panic("page directory has no mapping");
	regs->cr3 = cr3;
	
	/*! update the stack address*/
	thread_container->kernel_stack_pointer = (BYTE *) (((UINT32)regs) - sizeof(UINT32));
}

/*! Switches the execution context to the given thread
	\param thread_container - new thread container to switch to
*/
void SwitchContext(THREAD_CONTAINER_PTR thread_container)
{
	BYTE * esp;
	
	assert( thread_container->kernel_stack_pointer != 0);
	esp = thread_container->kernel_stack_pointer;
	/*reset the ring 0 stack pointer*/
	thread_container->kernel_stack_pointer = (BYTE *)PAGE_ALIGN_UP((UINT32)thread_container->kernel_stack_pointer);
	processor_i386[GetCurrentProcessorId()].tss.esp0 = (UINT32)thread_container->kernel_stack_pointer;
	
	asm volatile("movl %%eax, %%esp; jmp *%%ebx"
				:
				:"a"( esp ),
				 "b"( ReturnFromInterruptContext )
				);
}

/*! Invoke scheduler by generating timer interrupt*/
void InvokeScheduler()
{
	asm volatile("int $238"::);
}
