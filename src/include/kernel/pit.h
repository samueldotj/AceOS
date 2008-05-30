/*!
  \file		include/kernel/pit.h
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created: Thu May 29, 2008  10:11AM
  			Last modified: Thu May 29, 2008  11:14AM
  \brief	
*/


#ifndef _PIT_H_
#define _PIT_H_

/* Timer frequency is 100 Hz */
#define TIMER_FREQUENCY (100)

int InitPit(UINT32 frequency);
UINT32 ElapsedTicks();
void TimerSleep(UINT32 ticks);

#endif
