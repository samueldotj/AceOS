/*!
  \file		kernel/pit.h
  \brief	Programmable Interval Timer
*/

#ifndef _PIT_H_
#define _PIT_H_

#include <ace.h>
#include <kernel/interrupt.h>

/* Timer frequency is 100 Hz */
#define TIMER_FREQUENCY (100)

#define MILLISECONDS_TO_TICKS(ms)	((ms) / (1000/TIMER_FREQUENCY))
#define TICKS_TO_MILLISECONDS(ticks)	((ticks) * (1000/TIMER_FREQUENCY))

void InitPit(UINT32 frequency);
UINT32 ElapsedTicks();
void TimerSleep(UINT32 ticks);
void Delay(UINT32 ms);

ISR_RETURN_CODE _8254Handler(INTERRUPT_INFO_PTR interrupt_info, void * arg);
ISR_RETURN_CODE LapicTimerHandler(INTERRUPT_INFO_PTR interrupt_info, void * arg);

extern volatile UINT32 ElapsedSeconds;
extern volatile UINT32 timer_ticks;

#endif
