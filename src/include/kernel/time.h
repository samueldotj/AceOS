/*! \file 	kernel/time.h
    \brief 	timer and rtc related data structures and functions
*/

#include <ace.h>

#ifndef __TIME__H
#define __TIME__H

#define _SYSTIME_DEFINED
typedef struct system_time
{
    UINT16 year; 
    UINT16 month;
    UINT16 day_of_week;
    UINT16 day;
    UINT16 hour;
    UINT16 minute; 
    UINT16 second;
    UINT16 milliseconds; 
}SYSTEM_TIME, * SYSTEM_TIME_PTR; 

extern char month_short_names[12][4];

extern UINT32 lbolt;

#ifdef __cplusplus
    extern "C" {
#endif

UINT32 GetBootTime(SYSTEM_TIME_PTR pTime);
UINT32 GetLocalTime(SYSTEM_TIME_PTR pTime);
UINT32 SetLocalTime(SYSTEM_TIME_PTR pTime);

int InitRtc();

inline BYTE BcdToBinary(BYTE BcdValue);
inline BYTE BinaryToBcd(BYTE BinaryValue);
void BinaryTimeToBcdTime(SYSTEM_TIME_PTR pTime);
void BcdTimeToBinaryTime(SYSTEM_TIME_PTR pTime);

#ifdef __cplusplus
	}
#endif

#endif
