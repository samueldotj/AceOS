/*!
  \file		kernel/pit/pit.c
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created: Thu May 29, 2008  09:54AM
  			Last modified: Thu May 29, 2008  11:13AM
  \brief	Defines the interface for all PIT-related functions
*/


#include <ace.h>
#include <kernel/i386/interrupt.h>
#include <kernel/time.h>

extern void Init8254Timer(UINT32 frequency);

/*Static variables */
static UINT32 timer_ticks;

/* Static functions */
static void TimerHandler(struct regs *reg);


static void TimerHandler(struct regs *reg)
{
	timer_ticks++;
}

int InitPit(UINT32 frequency)
{
#if ARCH == i386
	InstallInterruptHandler(0, TimerHandler);
	Init8254Timer(frequency);
	/* Register our handler for taking care of IRQ0 */
	return 0;
#else
	return 1;
#endif
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
