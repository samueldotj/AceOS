/*!
  \file	kernel/i386/bios.c
  \brief	Functions to call BIOS 16bit code from protected mode using v86 mode
*/

#include <ace.h>
#include <kernel/debug.h>
#include <kernel/arch.h>
#include <kernel/pm/pm_types.h>
#include <kernel/pm/task.h>
#include <kernel/pm/thread.h>
#include <kernel/i386/i386.h>

/*! register content of last v86 thread which made int21 */
REGS_V86 last_v86_thread_context;

/*! Handles interrupts 21 raised from a v86 thread
	For now just terminate the thread after collecting the register information.
*/
void BiosInt21Handler(REGS_PTR reg)
{
	REGS_V86_PTR v86_reg = (REGS_V86_PTR) reg;
	memmove( &last_v86_thread_context, v86_reg, sizeof(REGS_V86) );
	ExitThread();
}

/*! Call 16bit BIOS interrupts from 32bit code
	\param interrupt - interrupt number to invoke
	\param input - register values for v86 mode
	\return register value during the exit of the v86 mode thread
*/
REGS_V86_PTR CallBiosIsr(BYTE interrupt, REGS_V86_PTR input)
{
	THREAD_I386 arg;
	VADDR entry_point;
	TASK_PTR task;
	UINT32 va=NULL;
	UINT16 program[2];
	THREAD_CONTAINER_PTR thread_container;
	
	/*Dynamically create a 16bit program 
		1) INT interrupt - to invoke the given ISR
		2) INT 13 - to collect the register values and terminate
	opcode for INT instruction is 0xCD*/
	program[0] = interrupt<<8 | 0xCD;		
	program[1] = 0x13<<8 | 0xCD;
	
	AllocateVirtualMemory( GetCurrentVirtualMap(), &va, 0, PAGE_SIZE, PROT_READ|PROT_WRITE, 0, NULL );
	memmove( (void *)va,(void *) program, sizeof(program)  );
	task = CreateTask((char *)va, IMAGE_TYPE_BIN_PROGRAM, TASK_CREATION_FLAG_NO_THREAD, &entry_point, NULL, NULL);
	if ( task == NULL )
	{
		KTRACE("Failed to create task");
		return NULL;
	}
		
	memset( &arg, 0, sizeof(arg) );
	/*create v86 thread*/
	arg.is_v86 = 1;
	arg.interrupt_handler = BiosInt21Handler;
	/*copy any register values given as input*/
	if( input != NULL )
		memmove( &arg.input_regs, input, sizeof(REGS_V86) );
	
	thread_container = CreateThread(task, (void *)I386LinearToFp(entry_point), SCHED_PRI_HIGH, FALSE, (VADDR)&arg);
	if( thread_container == NULL )
	{
		KTRACE("Failed to create thread");
		return NULL;
	}
	/*wait for the thread to terminate*/
	WaitForThread( &thread_container->thread, 0 );
	FreeThread( &thread_container->thread );
	
	return &last_v86_thread_context;
}

/*! Converts given 32bit linear address to 16bit far pointer(segment:offset)*/
UINT32 I386LinearToFp(UINT32 ptr)
{
    unsigned seg, off;
    off = ptr & 0xf;
    seg = ( ptr - ( ptr & 0xf)) / 16;
    return MK_FP(seg, off);
}
