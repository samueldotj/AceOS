/*!
	\file	kernel/i386/system_call_handler.c
	\brief	system call handler implementation
*/

#include <ace.h>
#include <kernel/system_call_handler.h>
#include <kernel/interrupt.h>
#include <kernel/mm/kmem.h>
#include <kernel/debug.h>
#include <heap/slab_allocator.h>

#define SYSTEM_CALL_INTERRUPT_NUMBER (0x80-32)

extern int dummy_system_call(SYSTEM_CALL_ARGS_PTR, UINT32*);

CACHE system_call_cache;


/* eax = system call number
 * ebx, ecx, edx, esi, edi contains 5 arguments 0-4.
 */
ISR_RETURN_CODE SystemCallHandler(INTERRUPT_INFO_PTR interrupt_info, void * arg)
{
	int sys_call_no;
	UINT32 *sys_ret_val;

	SYSTEM_CALL_ARGS_PTR sys_call_args;

	sys_call_no = interrupt_info->regs->eax;

	if(sys_call_no < 0 || sys_call_no >= max_system_calls)
		panic("SystemCallHandler: sys_call_no out of limit\n");

	sys_call_args = AllocateBuffer(&system_call_cache, 0); /* can sleep while memory is being allocated */

	sys_call_args->arg1 = interrupt_info->regs->ebx;
	sys_call_args->arg2 = interrupt_info->regs->ecx;
	sys_call_args->arg3 = interrupt_info->regs->edx;
	sys_call_args->arg4 = interrupt_info->regs->esi;
	sys_call_args->arg5 = interrupt_info->regs->edi;
	/*now call the required system call*/
	sys_ret_val = &(interrupt_info->regs->eax);
	interrupt_info->regs->ebx = (system_calls[sys_call_no])(sys_call_args, sys_ret_val); /* ebx will tell if system call succeeded(0) or not */

	return ISR_END_PROCESSING;
}


void SetupSystemCallHandler(void)
{
	InstallInterruptHandler(SYSTEM_CALL_INTERRUPT_NUMBER, &SystemCallHandler, 0); //interrupt no: 128

	/* Initialize the cache */
	if( InitCache(&system_call_cache, sizeof(SYSTEM_CALL_ARGS), SYSTEM_CALL_CACHE_FREE_SLABS_THRESHOLD, SYSTEM_CALL_CACHE_MIN_BUFFERS, SYSTEM_CALL_CACHE_MAX_SLABS, &SystemCallCacheConstructor, &SystemCallCacheDestructor) == -1)
		panic("SetupSystemCallHandler: Unable to initialize cache\n");
}
