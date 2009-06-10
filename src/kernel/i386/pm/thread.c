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
#include <kernel/i386/i386.h>

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
	\param is_kernel_thread - if one create kernel mode thread
	\param user_stack - stack for user mode part
	\param arch_arg - architecture depended argument(arch depended drivers can use - eg v86 mode driver)
*/
void FillThreadContext(THREAD_CONTAINER_PTR thread_container, void * start_address, BYTE is_kernel_thread, VADDR user_stack, VADDR arch_arg)
{
	UINT32 * stack_frame;
	REGS_PTR regs;
	UINT32 cr3;
	THREAD_I386_PTR i386_thread;
	THREAD_I386_PTR i386_arch_arg = (THREAD_I386_PTR)arch_arg;
	
	assert( thread_container != NULL );
	
	/*\todo - remove KMEM_NO_FAIL and handle kmalloc failure case or use cache allocator*/
	i386_thread = kmalloc( sizeof(THREAD_I386), KMEM_NO_FAIL );
	memset( i386_thread, 0, sizeof(THREAD_I386) );
	thread_container->thread.arch_data = i386_thread;
	
	/*build last stackframe and point it to ExitThread*/
	stack_frame = (UINT32 *) (thread_container->kernel_stack + sizeof(thread_container->kernel_stack));
	stack_frame[-2] = (UINT32)ExitThread;
	
	if ( i386_arch_arg && i386_arch_arg->is_v86 )
	{
		REGS_V86_PTR v86_reg;
		/*build exception frame*/
		v86_reg = (REGS_V86_PTR)( ((BYTE*)stack_frame) - sizeof(REGS_V86) );
		regs = &v86_reg->reg;
		
		v86_reg->v86.ds = v86_reg->v86.es = v86_reg->v86.fs = v86_reg->v86.gs =  FP_SEG(user_stack);
		v86_reg->v86.ds = i386_arch_arg->input_regs.v86.ds;
		v86_reg->v86.es = i386_arch_arg->input_regs.v86.es;
		v86_reg->v86.fs = i386_arch_arg->input_regs.v86.fs;
		v86_reg->v86.gs = i386_arch_arg->input_regs.v86.gs;
		
		regs->eax = i386_arch_arg->input_regs.reg.eax;
		regs->ebx = i386_arch_arg->input_regs.reg.ebx;
		regs->ecx = i386_arch_arg->input_regs.reg.ecx;
		regs->edx = i386_arch_arg->input_regs.reg.edx;
		regs->edi = i386_arch_arg->input_regs.reg.edi;
		regs->esi = i386_arch_arg->input_regs.reg.esi;
		regs->ebp = i386_arch_arg->input_regs.reg.ebp;
		
		regs->cs = FP_SEG(start_address);
		regs->eip = FP_OFF(start_address);
		regs->ds = regs->es = regs->gs = regs->fs = USER_CODE_SELECTOR | USER_PRIVILEGE_LEVEL;
		regs->eflags = EFLAG_VALUE | EFLAG_VM86 | EFLAG_VIF | EFLAG_IOPL3;
		regs->ss = FP_SEG(user_stack);
		regs->useresp = FP_OFF(user_stack);		
		
		i386_thread->is_v86 = 1;
		i386_thread->interrupt_handler = i386_arch_arg->interrupt_handler;
	}
	else
	{
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
			regs->useresp = user_stack+USER_STACK_SIZE-1;
		}	
		regs->eip = (UINT32)start_address;
		regs->eflags = EFLAG_VALUE;
	}

	/*! update the stack address*/
	thread_container->kernel_stack_pointer = (BYTE *) (((UINT32)regs) - sizeof(UINT32));
		
	/*get the physical address of the page directory*/
	if( TranslatePaFromVa((VADDR)thread_container->thread.task->virtual_map->physical_map->page_directory, &cr3) == VA_NOT_EXISTS )
		panic("page directory has no mapping");
	regs->cr3 = cr3;
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
