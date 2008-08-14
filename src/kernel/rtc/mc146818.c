/*! \file kernel/rtc/mc146818.c
    \brief PC Real Time Clock MC146818 chip access
  
*/
#include <ace.h>
#include <kernel/io.h>
#include <kernel/time.h>

/*Command registers*/
#define MC146818_INDEX_REG		0x70
#define MC146818_VALUE_REG		0x71

/*status registers*/
#define MC146818_STATUS_A		0xA
#define MC146818_STATUS_B		0xB
#define MC146818_STATUS_C		0xC
#define MC146818_STATUS_D		0xD

/*Data register index*/
#define MC146818_SECOND 		0
#define MC146818_SECOND_ALARAM	1
#define MC146818_MINUTE 		2
#define MC146818_MINUTE_ALARAM	3
#define MC146818_HOUR   		4
#define MC146818_HOUR_ALARAM	5
#define MC146818_DAY_OF_WEEK  	6
#define MC146818_DAY   			7
#define MC146818_MONTH 			8
#define MC146818_YEAR  			9

static int status_reg_b_value;

#define	DAYLIGHT_ENABLED_IN_MC146818	(status_reg_b_value & 0x1 )
#define	HOUR_24_ENABLED_IN_MC146818		(status_reg_b_value & 0x2 )
#define	BCD_ENABLED_IN_MC146818			(!(status_reg_b_value & 0x4 ))


static void WriteToMC146818(BYTE reg, BYTE value)
{
	_outp( MC146818_INDEX_REG, reg );
	_outp( MC146818_VALUE_REG, value );
}
static BYTE ReadFromMC146818(BYTE reg)
{
	_outp( MC146818_INDEX_REG, reg );
	return _inp( MC146818_VALUE_REG);
}

void InitMC146818()
{
	status_reg_b_value = ReadFromMC146818( MC146818_STATUS_B );
	status_reg_b_value &= 0x7; //disable interrupts
	WriteToMC146818( MC146818_STATUS_B, status_reg_b_value);
}

static inline int IsDeviceReady()
{
	int time_out = 1000;
	while ( ReadFromMC146818(MC146818_STATUS_A) & 0x80 ) /*if high bit is set then update in progress*/
    {
        if ( time_out <= 0 )
            return FALSE;
		time_out--;
	}
	
	return TRUE;
}
/*! gets the system date and time from RTC .

	This function is called only once by kernel during booting.
	After that whenever the kernel requires date and time information it uses
	the cached copy.
	
	\param pSystemTime - system time structure to update the value
	\return 0 on success 
			1 on failure.
	\note This function also disables any interrupt from RTC chip.
*/
UINT32 GetLocalTimeFromMC146818(SYSTEM_TIME_PTR pSystemTime)
{
	if ( !IsDeviceReady() )
		return 1;
    /*get date values*/
    pSystemTime->day = ReadFromMC146818( MC146818_DAY );
    pSystemTime->month = ReadFromMC146818( MC146818_MONTH ) ;
    pSystemTime->year = ReadFromMC146818( MC146818_YEAR );
    pSystemTime->day_of_week = ReadFromMC146818( MC146818_DAY_OF_WEEK ) ;
	/*get time values*/
    pSystemTime->second = ReadFromMC146818( MC146818_SECOND ) ;
    pSystemTime->minute = ReadFromMC146818( MC146818_MINUTE ) ;
    pSystemTime->hour = ReadFromMC146818( MC146818_HOUR );
	
	//convert if it is in bcd form
	if ( BCD_ENABLED_IN_MC146818 )
		BcdTimeToBinaryTime(pSystemTime);
    return 0;
}
/*! sets the system time 
	\param pSystemTime - system time structure to update the value
	\return 0 on success 
			1 on failure.
	\note This function also disables any interrupt from RTC chip.
*/
UINT32 SetLocalTimeToMC146818(SYSTEM_TIME_PTR pSystemTime)
{
	if ( !IsDeviceReady() )
		return 1;
	
	/*set the values*/
	WriteToMC146818(MC146818_DAY, BinaryToBcd(pSystemTime->day) );
	WriteToMC146818(MC146818_MONTH, BinaryToBcd(pSystemTime->month) ); 
	WriteToMC146818(MC146818_YEAR, BinaryToBcd(pSystemTime->year) ); 
	
	WriteToMC146818(MC146818_SECOND, BinaryToBcd(pSystemTime->second) ); 
	WriteToMC146818(MC146818_MINUTE, BinaryToBcd(pSystemTime->minute) ); 
	WriteToMC146818(MC146818_HOUR, BinaryToBcd(pSystemTime->hour) ); 

	//convert if needed
	if ( BCD_ENABLED_IN_MC146818 )
		BinaryTimeToBcdTime(pSystemTime);
		
	return 0;
}

