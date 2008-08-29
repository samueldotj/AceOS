/*!
  \file		kernel/pit.h
  \brief	Programmable Interval Timer
*/


#ifndef _PIT_H_
#define _PIT_H_

/* Timer frequency is 100 Hz */
#define TIMER_FREQUENCY (100)

int InitPit(UINT32 frequency);
UINT32 ElapsedTicks();
void TimerSleep(UINT32 ticks);
void Delay(UINT32 ms);

#endif
