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

#define CPUID_MAX_STD_LEVELS			0x0A
#define CPUID_MAX_EXT_LEVELS			0x1A

#define CPUID_MAX_CACHE_CONFIGURATION	0x3

enum
{
	CPUID_STD_VENDOR_ID = 0,
	CPUID_STD_PROCESSOR_TYPE,
	CPUID_STD_PROCESSOR_CONFIGURATION_DESCRIPTORS,
	CPUID_STD_PROCESSOR_SERIAL_NUMBER,
	CPUID_STD_CACHE_CONFIGURATION_DESCRIPTORS,
	CPUID_STD_MONITOR_INFORMATION,
	CPUID_STD_POWER_MANAGEMENT_INFORMATION,
	CPUID_STD_DCA_PARAMETER,
	CPUID_STD_PERFORMANCE_MONITOR_INFORMATION,
	CPUID_STD_TOPOLOGY_ENUMERATION_INFORMATION,
	CPUID_STD_EXTENDED_STATE_ENUMERATION
}CPUID_LEVELS;

typedef struct cpuid_result
{
	UINT32	eax, ebx, ecx, edx;
}CPUID_RESULT, * CPUID_RESULT_PTR;

typedef struct cpuid_info
{
	union
	{
		CPUID_RESULT	raw;
		struct
		{
			UINT32	max_std_level;
			char 	vendor_id[3*sizeof(UINT32)];
		}_;
	}basic;
    
	union
	{
		CPUID_RESULT	raw;
	    struct
		{
			UINT32	
				stepping:4,
				model:4,
				family:4,
				type:2,
				reserved_1:2,
				extended_model:4,
				extended_family:8,
				reserved_2:4;
			UINT32
				brand_id:8,
				clflush:8,
				logical_processor_count:8,
				apic_id:8;
			UINT32  
				sse3:1,
				pclmulqdq:1,
				debug_store_64bit:1,
				monitor_mwait:1,
				ds_cpl:1,
				virtual_machine_extn:1,
				safer_mode_extn:1,
				eist:1,
				thermal_monitor:1,
				ssse3:1,
				context_id:1,
				fma:1,
				cmpxchg_16bytes:1,
				send_task_priority_msg:1,
				perfromance_capability_msr:1,
				reserved_3:2,
				direct_cache_access:1,
				reserved_4:13;
			UINT32  
				fpu_on_chip: 1,
				virtual_mode_extension:1,
				debugging_extension:1,
				page_size_extension:1,
				time_stamp_counter:1,
				model_specific_registers:1,
				physical_address_extension:1,
				machine_check_extension:1,
				cmpxchg8:1,
				apic:1,
				reserved_5:1,
				sysenter:1,
				memory_type_range_registers:1,
				page_global_enable:1,
				machine_check_architecture:1,
				conditional_move_instruction:1,
				page_attribute_table:1,
				page_size_extension_32bit:1,
				processor_serial_number:1,
				clflush_instruction:1,
				reserved_6:1,
				dte:1,
				acpi_thermal_control_msr:1,
				mmx:1,
				fast_floating_point:1,
				sse:1,
				sse2:1,
				self_snoop:1,
				hyper_threading_technology:1,
				thermal_interrupt:1,
				ia64:1,
				pending_break_enable:1;
		}_;
	}feature;
	
	union
	{
		CPUID_RESULT raw;
		char string[4*sizeof(UINT32)];
	}processor_serial_number;
	
	union
	{
		CPUID_RESULT raw;
		struct
		{
			UINT32
				cache_type:5,
				cache_level:3,
				self_initializing:1,
				fully_associative:1,
				reserved_1:4,
				threads_per_cache:10,
				cores_per_package:6;
			UINT32
				system_coherency_line_size:12,
				physical_line_partitions:10,
				ways_of_associativity:10;
			UINT32
				sets;
			UINT32
				write_back_invalidate:1,
				inclusive_of_lower_levels:1,
				reserved_2:30;
		}_;
	}cache_configuration_descriptor[CPUID_MAX_CACHE_CONFIGURATION];
	
	union
	{
		CPUID_RESULT raw;
		struct
		{
			UINT32
				smallest_monitor_line_size:16,
				reserved_1:16;
			UINT32
				largest_monitor_line_size:16,
				reserved_2:16;
			UINT32
				mwait_extensions:1,
				break_on_interrupt:1,
				reserved_3:30;
			UINT32
				c0_substates:4,
				c1_substates:4,
				c2_substates:4,
				c3_substates:4,
				c4_substates:4,
				reserved_4:12;
		}_;
	}monitor_information;
	
	union
	{
		CPUID_RESULT raw;
		struct
		{
			UINT32
				digital_thermometer:1,
				dynamic_acceleration_enabled:1,
				operating_point_protection:1,
				reserved_1:29;
			UINT32
				thermometer_interrupt_thresholds:4,
				reserved_2:28;
			UINT32
				acnt:1,
				reserved_3:31;
			UINT32
				reserved_4;
		}_;
	}power_management_information;
	
	union
	{
		CPUID_RESULT raw;
		struct
		{
			UINT32 platform_dca_cap;
			UINT32 reserved_1;
			UINT32 reserved_2;
			UINT32 reserved_3;
		}_;
	}dca_information;
	
	union
	{
		CPUID_RESULT raw;
		struct
		{
			UINT32
				revision:8,
				counters_per_logical_processors:8,
				bit_width:8,
				ebx_bit_vector_length:8;
			UINT32
				core_cycles_event_unavailable:1,
				instructions_retired_event_unavailable:1,
				reference_cycles_event_unavailable:1,
				last_level_cache_references_event_unavailable:1,
				last_level_cache_misses_event_unavailable:1,
				branch_instructions_retired_event_unavailable:1,
				branch_mispredicts_retired_event_unavailable:1,
				reserved_1:25;
			UINT32
				reserved_2;
			UINT32
				fixed_function_performance_monitor_counters:5,
				fixed_function_performance_monitor_counter_bit_width:8,
				reserved_3:19;
		}_;
	}performance_monitor_information;
	
}CPUID_INFO, * CPUID_INFO_PTR;

extern CPUID_INFO cpuid_info;
void LoadCpuIdInfo();

#endif
