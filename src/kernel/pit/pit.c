/*!
  \file		kernel/pit/pit.c
  \brief	Defines the interface for all PIT-related functions
*/


#include <ace.h>
#include <kernel/time.h>
#include <kernel/pit.h>
#include <kernel/i386/ioapic.h>
#include <kernel/interrupt.h>
#include <kernel/pm/task.h>

extern THREAD_CONTAINER_PTR kthread1, kthread2;

/*! Timer tick - relative to the last tick - used for tickless system*/
static volatile UINT32 timer_ticks;

/* Elapsed seconds since boot - used for time keeping*/
volatile UINT32 ElapsedSeconds = 0;

/*! This function is called for each timer tick generated by 8254.
*/
ISR_RETURN_CODE _8254Handler(INTERRUPT_INFO_PTR interrupt_info, void * arg)
{
	timer_ticks++;
	return ISR_END_PROCESSING;
}

/*! This function is called for each timer tick generated by the LAPIC timer.
	For i386 arch this is the function is called for when a new thread's quantum expires
	
	\todo - Modify scheduler code to use variable quantum frequency and StartTimer with single shot mode
*/
ISR_RETURN_CODE LapicTimerHandler(INTERRUPT_INFO_PTR interrupt_info, void * arg)
{
	THREAD_PTR current_thread = GetCurrentThread();
	THREAD_CONTAINER_PTR tc = STRUCT_ADDRESS_FROM_MEMBER( current_thread, THREAD_CONTAINER, thread );
	
	/*Update current thread's kernel stack pointer*/
	tc->kernel_stack_pointer = (BYTE *) ((UINT32) interrupt_info->regs) - sizeof(UINT32);
	
	/*Send EOI to the PIC*/
	SendEndOfInterrupt( interrupt_info->interrupt_number);
	
	/*Select new thread to run*/
	ScheduleThread( current_thread );
	
	return ISR_END_PROCESSING;
}

/*! Pauses the execution by the given number of milli seconds
 * \param ms - Miliseconds to pause
 */
void Delay(UINT32 ms)
{
	UINT32 ticks_to_go = timer_ticks + (ms / (1000/TIMER_FREQUENCY));
	while(timer_ticks < ticks_to_go);
}
