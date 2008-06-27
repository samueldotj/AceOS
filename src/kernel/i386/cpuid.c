/*!
  \file		cpuid.c
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created: Sun Jun 15, 2008  12:23PM
  			Last modified: Tue Jun 17, 2008  03:05PM
  \brief	Provides support for cpu identification.
*/

#include <ace.h>
#include <string.h>
#include <kernel/i386/cpuid.h>
#include <ds/bits.h>

CPUID_INFO cpuid_info[MAX_PROCESSORS];

static inline void ExecuteCpuId(CPUID_RESULT_PTR raw)
{
	__asm__ volatile("cpuid"
		: 
			"=a" ( raw->eax ),
			"=b" ( raw->ebx ),
			"=c" ( raw->ecx ),
			"=d" ( raw->edx )
		:
			"a" ( raw->eax ),
			"c" ( raw->ecx )
		);
}
void LoadCpuIdInfo(CPUID_INFO_PTR cpuid_info)
{
	int i,j;
	UINT32 tmp;
	
	/*initalize the data*/
	memset( cpuid_info, 0, sizeof(CPUID_INFO) );
	
	/*find the max level and vendor id*/
	cpuid_info->basic.raw.eax = CPUID_STD_VENDOR_ID;
	ExecuteCpuId( &cpuid_info->basic.raw );
	
	/*correct the vendor string*/
	tmp = cpuid_info->basic.raw.ecx;
	cpuid_info->basic.raw.ecx = cpuid_info->basic.raw.edx;
	cpuid_info->basic.raw.edx = tmp;
	
	/*loop and load all levels*/
	for(i=1; i <= cpuid_info->basic._.max_std_level && i <= CPUID_MAX_STD_LEVELS; i++ )
	{
		CPUID_RESULT_PTR data;
		switch(i)
		{
			case CPUID_STD_PROCESSOR_TYPE:
				data = &cpuid_info->feature.raw;
				break;
			case CPUID_STD_PROCESSOR_SERIAL_NUMBER:
				data = &cpuid_info->processor_serial_number.raw;
				break;
			case CPUID_STD_CACHE_CONFIGURATION_DESCRIPTORS:
				for (j=0;j<CPUID_MAX_CACHE_CONFIGURATION;j++)
				{
					data = &cpuid_info->cache_configuration_descriptor[j].raw;
					cpuid_info->basic.raw.eax = i;
					cpuid_info->basic.raw.ecx = j;
					ExecuteCpuId( data );
				}
				continue;
			case CPUID_STD_MONITOR_INFORMATION:
				data = &cpuid_info->monitor_information.raw;
				break;
			case CPUID_STD_POWER_MANAGEMENT_INFORMATION:
				data = &cpuid_info->power_management_information.raw;
				break;
			case CPUID_STD_DCA_PARAMETER:
				data = &cpuid_info->dca_information.raw;
				break;
			case CPUID_STD_PERFORMANCE_MONITOR_INFORMATION:
				data = &cpuid_info->performance_monitor_information.raw;
				break;
			default:
				continue;
		};
		data->eax = i;
		ExecuteCpuId( data );
	}
	kprintf("%s cpuid %d stepping %d model %d family %d\n", 
							cpuid_info->basic._.vendor_id, 
							cpuid_info->basic._.max_std_level,
							cpuid_info->feature._.stepping, 
							cpuid_info->feature._.model,
							cpuid_info->feature._.family);
	kprintf("%X brand Id %d logical processor count %d SSE %d\n", cpuid_info->feature.raw.ebx, cpuid_info->feature._.brand_id, CPU_LOGICAL_PROCESSOR_COUNT(0), CPU_FEATURE_SSE(0) );
}
