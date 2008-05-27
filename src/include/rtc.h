/*! \file kernel/rtc.h
    \brief timer and rtc related data structures and functions
	\author Samuel 
    \date 26-May-08 4:06PM
*/
#include <ace.h>
#include <time.h>

#ifndef __RTC__H
#define __RTC__H

#ifdef __cplusplus
    extern "C" {
#endif

int InitRtc();

static void TimeAdd(SYSTEM_TIME_PTR pTime1, SYSTEM_TIME_PTR pTime2);
static void TicksToTime(UINT32 ticks, SYSTEM_TIME_PTR pTime);
inline BYTE BcdToBinary(BYTE BcdValue);
inline BYTE BinaryToBcd(BYTE BinaryValue);

#ifdef __cplusplus
	}
#endif


#endif