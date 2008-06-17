/*!
  \file		kernel/i386/cpuid.h
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created: Sat Jun 14, 2008  06:18PM
  			Last modified: Tue Jun 17, 2008  03:04PM
  \brief	
*/


#ifndef _CPUID_H_
#define _CPUID_H_
#include <ace.h>

typedef union feature_flags_edx
{
	UINT32 feature_flags; /* Placeholder to split the integer into bits */
	struct
	{
		UINT32 fpu_on_chip: 1; //0
		UINT32 virtual_mode_extn: 1; //1
		UINT32 debugging_exten: 1; //2
		UINT32 page_size_exten: 1; //3
		UINT32 time_stamp_counter: 1; //4
		UINT32 model_specific_registers: 1; //5
		UINT32 physical_address_exten: 1; //6
		UINT32 machine_check_exten: 1; //7
		UINT32 cmpxchg8: 1; //8
		UINT32 apic : 1; //9
		UINT32 reserved1 : 1; //10
		UINT32 fast_system_call : 1; //11
		UINT32 memory_type_range_registers : 1; //12
		UINT32 page_global_enable : 1; //13
		UINT32 machine_check_architecture : 1; //14
		UINT32 conditional_move_instr : 1; //15
		UINT32 page_attribute_table : 1; //16
		UINT32 page_size_extn_32bit : 1; //17
		UINT32 processor_serial_number : 1; //18
		UINT32 clflush_instr : 1; //19
		UINT32 reserved2 : 1; //20
		UINT32 debug_store : 1; //21
		UINT32 acpi : 1; //22
		UINT32 mmx : 1; //23
		UINT32 fast_floating_point : 1; //24
		UINT32 sse : 1; //streaming SIMD extension //25
		UINT32 sse2 : 1; //26
		UINT32 self_snoop : 1; //27
		UINT32 multi_threading : 1; //28
		UINT32 thermal_monitor : 1; //29
		UINT32 ia64 : 1; //30
		UINT32 pending_break_enable : 1; //31
	};
}FEATURE_FLAGS_EDX, * FEATURE_FLAGS_EDX_PTR;


typedef union feature_flags_ecx
{
	UINT32 feature_flags;  /* Placeholder to split the integer into bits */
	struct
	{
		UINT32  sse3:1 ; //0
		UINT32  reserved1:1 ; //1
		UINT32  debug_store_64bit:1 ; //2
		UINT32  monitor_mwait:1 ; //3
		UINT32  ds_cpl:1 ; //CPL qualified debug store //4
		UINT32  virtual_machine_extn:1 ; //5
		UINT32  safer_mode_extn:1 ; //6
		UINT32  est:1 ; //enhanced Intel SpeedStep technology //7
		UINT32  thermal_monitor2:1 ; //8
		UINT32  ssse3:1 ; //Supplemental Streaming SIMD Extensions 3 //9
		UINT32  context_id:1 ; //10
		UINT32  reserved2:2 ; //11,12
		UINT32  cmpxchg_16bytes:1 ; //13
		UINT32  send_task_priority_msg:1 ; //14
		UINT32  perfromance_capability_msr:1 ; //15
		UINT32  reserved3:2 ; //16,17
		UINT32  direct_cache_access:1 ; //18
		UINT32  reserved4:13 ; //19-31
	};
}FEATURE_FLAGS_ECX, * FEATURE_FLAGS_ECX_PTR;

typedef struct cpuid_info
{
    UINT32 eax_max;
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
    UINT32 apic_id;
	FEATURE_FLAGS_EDX feature_flags_edx;
	FEATURE_FLAGS_ECX feature_flags_ecx;
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
