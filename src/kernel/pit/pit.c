/*!
  \file		kernel/pit/pit.c
  \brief	Defines the interface for all PIT-related functions
*/


#include <ace.h>
#include <kernel/interrupt.h>
#include <kernel/time.h>

extern void Init8254Timer(UINT32 frequency);

/*Static variables */
static UINT32 timer_ticks;

ISR_RETURN_CODE TimerHandler (INTERRUPT_INFO_PTR interrupt_info, void * arg)
{
	timer_ticks++;
	return ISR_END_PROCESSING;
}

int InitPit(UINT32 frequency)
{
	InstallInterruptHandler(0, TimerHandler, 0);
#if ARCH == i386
	Init8254Timer(frequency);
	return 0;
#endif
	return 1;
}

void TimerSleep(UINT32 ticks)
{
	UINT32 final_ticks; 

	final_ticks = timer_ticks + ticks;
	while(timer_ticks < final_ticks);
}

UINT32 ElapsedTicks()
{
	return timer_ticks;
}
