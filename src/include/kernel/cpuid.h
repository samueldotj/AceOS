/*!
  \file		cpuid.h
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created: Sat Jun 14, 2008  06:18PM
  			Last modified: Mon Jun 16, 2008  02:40PM
  \brief	
*/


#ifndef _CPUID_H_
#define _CPUID_H_
#include <ace.h>

typedef struct cpuid_info
{
    UINT32 eax_max;
    UINT32 apic_id;
    char vendor_id[20];
    UINT32 stepping_id;
    UINT32 model_number;
    UINT32 family_code;
    UINT32 processor_type;
    UINT32 extended_model;
    UINT32 extended_family;
    UINT32 brand_id;
    UINT32 cache_line_size;
    UINT32 count_logical_processors;
    UINT32 bit_array;
    UINT32 serial_number_low;
    UINT32 serial_number_high;
    UINT32 processor_label;
	UINT32 local_apic_present;
}CPUID_INFO, * CPUID_INFO_PTR;


enum cpuid_command
{
    EAX_MAX,
    APIC_ID,
    VENDOR_ID,
    STEPPING_ID,
    MODEL_NUMBER,
    FAMILY_CODE,
    PROCESSOR_TYPE,
    EXTENDED_MODEL,
    EXTENDED_FAMILY,
    BRAND_ID,
    CACHE_LINE_SIZE,
    COUNT_LOGICAL_PROCESSORS,
    BIT_ARRAY,
    SERIAL_NUMBER_LOW,
    SERIAL_NUMBER_HIGH,
    PROCESSOR_LABEL,
	LOCAL_APIC_PRESENT
};

void LoadDataStructureFromCpuID();
void * GetFromCpuId(int command);
int DetectCpuId(void);

#endif
