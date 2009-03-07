/*!
	\file	kernel/i386/system_call_handler.c
	\brief	i386 specific system call handler implementation
*/

#include <ace.h>
#include <ds/bits.h>
#include <kernel/debug.h>
#include <kernel/system_call_handler.h>
#include <kernel/interrupt.h>
#include <kernel/mm/kmem.h>

#define SYSTEM_CALL_INTERRUPT_NUMBER (0x80-32)

/*! Handles the system call interrupt
	\param interrupt_info - passed by the generic ISR code - contains the register state at the time of system call raised
	\param arg - not used
	\note The following registers are used as input
			eax = system call number
			ebx, ecx, edx, esi, edi contains 5 arguments 0-4.
		   The following registers are used as output
			eax = return value
			ebx = error code
 */
ISR_RETURN_CODE SystemCallHandler(INTERRUPT_INFO_PTR interrupt_info, void * arg)
{
	int sys_call_no;
	SYSTEM_CALL_ARGS sys_call_args;

	sys_call_no = interrupt_info->regs->eax;
	
	if( !VALUE_WITH_IN_RANGE(0, max_system_calls, sys_call_no) )
	{
		KTRACE("sys_call_no out of limit %d\n", sys_call_no);
		return ISR_END_PROCESSING;
	}		

	sys_call_args.args[0] = interrupt_info->regs->ebx;
	sys_call_args.args[1] = interrupt_info->regs->ecx;
	sys_call_args.args[2] = interrupt_info->regs->edx;
	sys_call_args.args[3] = interrupt_info->regs->esi;
	sys_call_args.args[4] = interrupt_info->regs->edi;

	/*now call the required system call*/
	interrupt_info->regs->ebx = (system_calls[sys_call_no])(&sys_call_args, &interrupt_info->regs->eax); /* ebx will tell if system call succeeded(0) or not */

	return ISR_END_PROCESSING;
}

void SetupSystemCallHandler(void)
{
	InstallInterruptHandler(SYSTEM_CALL_INTERRUPT_NUMBER, &SystemCallHandler, 0);
}
