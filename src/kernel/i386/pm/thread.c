/*!
  \file	kernel/i386/pm/thread.c
  \brief	i386 specific thread management routines
*/

#include <ace.h>
#include <kernel/pm/task.h>
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
void FillThreadContext(THREAD_CONTAINER_PTR thread_container, void * start_address)
{
	UINT32 * stack_frame;
	REGS_PTR regs;
	
	assert( thread_container != NULL );
	
	/*build last stackframe and point it to ExitThread*/
	stack_frame = (UINT32 *) (thread_container->kernel_stack + sizeof(thread_container->kernel_stack));
	stack_frame[-2] = (UINT32)ExitThread;
	
	/*build exception frame*/
	regs = (REGS_PTR)( ((BYTE*)stack_frame) - sizeof(REGS) );
	regs->cs = KERNEL_CODE_SELECTOR;
	regs->eip = (UINT32)start_address;
	regs->ds = regs->es = regs->gs = regs->fs = regs->ss = KERNEL_DATA_SELECTOR;
	regs->eflags = EFLAG_VALUE;
	
	thread_container->kernel_stack_pointer = (BYTE *) ((UINT32)regs) - sizeof(UINT32);
}

/*! Switches the execution context to the given thread
	\param thread_container - new thread container to switch to
*/
void SwitchContext(THREAD_CONTAINER_PTR thread_container)
{
	asm volatile("movl %%eax, %%esp; jmp *%%ebx"
				:
				:"a"( thread_container->kernel_stack_pointer ),
				 "b"( ReturnFromInterruptContext )
				);
}

/*! Invoke scheduler by generating timer interrupt*/
void InvokeScheduler()
{
	asm volatile("int $238"::);
}
