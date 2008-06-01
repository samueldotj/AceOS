/*! \file kernel/rtc/rtc.c
	\author		Samuel
	\version	3.0
	\date	
  			Created: 26-May-08 4:00PM
  			Last modified: 26-May-08 4:00PM
    \brief Real Time Clock programming
	
	RTC chip keeps the date and time while your computer is turned off. 
	However IO access to RTC slow, so Ace kernel uses RTC to get current date and 
	time only once. After that PIT should up date it.
*/

#include <kernel/time.h>
#include <string.h>

void InitMC146818();
UINT32 GetLocalTimeFromMC146818(SYSTEM_TIME_PTR pSystemTime);
UINT32 SetLocalTimeToMC146818(SYSTEM_TIME_PTR pSystemTime);
static void TimeAdd(SYSTEM_TIME_PTR pTime1, SYSTEM_TIME_PTR pTime2);
static void TicksToTime(UINT32 ticks, SYSTEM_TIME_PTR pTime);

/*! boot_time - filled by RTC chips during booting.*/
static SYSTEM_TIME boot_time;

int InitRtc()
{
	memset( &boot_time, 0, sizeof(SYSTEM_TIME) );
#if ARCH == i386
	InitMC146818();
	return GetLocalTimeFromMC146818(&boot_time);
#else
	return 1;
#endif
}

/*! gets boot time of the system
	\param pTime - time structure to fill the boot time
	\return 0 on success
			1 on failure
*/
UINT32 GetBootTime(SYSTEM_TIME_PTR pTime)
{
	memmove(pTime, &boot_time, sizeof(SYSTEM_TIME) );
	return 0;
}
/*! gets the current system time
	\param pTime - time structure to fill with the local time
	\return 0 on success
			1 on failure
*/
UINT32 GetLocalTime(SYSTEM_TIME_PTR pTime)
{
	SYSTEM_TIME pElapsedTime;
	
	memmove(pTime, &boot_time, sizeof(SYSTEM_TIME) );

	//convert the elapsed ticks to time format
	TicksToTime(ElapsedTicks(), &pElapsedTime);
	
	//add the elapsed time to boot_time
	TimeAdd( pTime, &pElapsedTime);
	
	return 0;
}
/*! sets the system time
	\param pTime - time structure to set 
	\return 0 on success
			1 on failure
*/
UINT32 SetLocalTime(SYSTEM_TIME_PTR pTime)
{
#if ARCH == i386
	return SetLocalTimeToMC146818(pTime);
#endif
}

/*! converts the ticks into time structure.
*/
static void TicksToTime(UINT32 ticks, SYSTEM_TIME_PTR pTime)
{
	memset(pTime, 0, sizeof(SYSTEM_TIME) );
	pTime->second = ticks % 60;
	pTime->minute = (ticks % (60*60)) / 60;
	pTime->hour = (ticks % (24*60*60)) / (60*60);
	pTime->day = ticks / (24*60*60);
}
/*! adds the pTime1 to pTime2 and stores the result in pTime1
*/
static void TimeAdd(SYSTEM_TIME_PTR pTime1, SYSTEM_TIME_PTR pTime2)
{
	pTime1->second += pTime2->second;
	if ( pTime1->second  % 60 )
	{
		pTime1->minute += 1;
		pTime1->second %= 60;
	}
	pTime1->minute += pTime2->minute;
	if ( pTime1->minute  % 60 )
	{
		pTime1->hour += 1;
		pTime1->minute  %= 60;
	}
	pTime1->hour += pTime2->hour;
	if ( pTime1->hour  % 24 )
	{
		pTime1->day += 1;
		pTime1->hour  %= 24;
	}
}

/*! converts bcd to binary and bcd to binary*/
inline BYTE BcdToBinary(BYTE BcdValue)
{
	return ((((BcdValue >> 4) & 0x0F) * 10) + (BcdValue & 0x0F));
}
/*! converts bcd to binary and bcd to binary*/
inline BYTE BinaryToBcd(BYTE BinaryValue)
{
	return (((BinaryValue / 10) << 4) | (BinaryValue % 10));
}

void BinaryTimeToBcdTime(SYSTEM_TIME_PTR pTime)
{
	pTime->day = BinaryToBcd(pTime->day);
	pTime->month = BinaryToBcd(pTime->month);
	pTime->year = BinaryToBcd(pTime->year);
	pTime->day_of_week = BinaryToBcd(pTime->day_of_week);
	pTime->minute = BinaryToBcd(pTime->minute);
	pTime->second = BinaryToBcd(pTime->second);
	pTime->hour = BinaryToBcd(pTime->hour);
}
void BcdTimeToBinaryTime(SYSTEM_TIME_PTR pTime)
{
	pTime->day = BcdToBinary(pTime->day);
	pTime->month = BcdToBinary(pTime->month);
	pTime->year = BcdToBinary(pTime->year);
	pTime->day_of_week = BcdToBinary(pTime->day_of_week);
	pTime->minute = BcdToBinary(pTime->minute);
	pTime->second = BcdToBinary(pTime->second);
	pTime->hour = BcdToBinary(pTime->hour);
}
