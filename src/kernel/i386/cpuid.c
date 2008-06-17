/*!
  \file		cpuid.c
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created: Sun Jun 15, 2008  12:23PM
  			Last modified: Tue Jun 17, 2008  09:03AM
  \brief	Provides support for cpu identification.
*/

#include <ace.h>
#include <string.h>
#include <kernel/i386/cpuid.h>

static CPUID_INFO cpuid_info;



/*!
	\brief	Loads the cpuid_info structure with all available cpuid information. 

	\param	 

	\return	 void
*/
void LoadDataStructureFromCpuID()
{
	/* call CPUID with all possible values and store all the required information */
	/* After this call, we can just read value from data structures without execting CPUID instruction every time */
}



/*!
	\brief	Returns the value of the attribute requested from cpuid_info data structure. 

	\param	 command: Atrribute in cpuid.

	\return	 NULL: Failure
			 void pointer: Value of the corresponding attribute from cpuid.
*/
void * GetFromCpuId(int command)
{
	switch(command)
	{
		case EAX_MAX					: return &(cpuid_info.eax_max);
		case APIC_ID					: return &(cpuid_info.apic_id);
		case VENDOR_ID					: return &(cpuid_info.vendor_id);
		case STEPPING_ID				: return &(cpuid_info.stepping_id);
		case MODEL_NUMBER				: return &(cpuid_info.model_number);
		case FAMILY_CODE				: return &(cpuid_info.family_code);
		case PROCESSOR_TYPE				: return &(cpuid_info.processor_type);
		case EXTENDED_MODEL				: return &(cpuid_info.extended_model);
		case EXTENDED_FAMILY			: return &(cpuid_info.extended_family);
		case BRAND_ID					: return &(cpuid_info.brand_id);
		case CACHE_LINE_SIZE			: return &(cpuid_info.cache_line_size);
		case COUNT_LOGICAL_PROCESSORS	: return &(cpuid_info.count_logical_processors);
		case BIT_ARRAY					: return &(cpuid_info.bit_array);
		case SERIAL_NUMBER_LOW			: return &(cpuid_info.serial_number_low);
		case SERIAL_NUMBER_HIGH			: return &(cpuid_info.serial_number_high);
		case PROCESSOR_LABEL			: return &(cpuid_info.processor_label);
		case LOCAL_APIC_PRESENT			: return &(cpuid_info.local_apic_present);
		default							: return NULL;/* Invalid command */
	};
}

/*!
	\brief	Check if CPUID instruction is available on the system. 

	\param	 

	\return	 0: CPUID is present 
			-1: CPUID is NOT present.
*/

int DetectCpuId(void)
{
	char *vendor_id;
	/* If write to bit 21 of EFLAGS register is possible, then CPUID instruction is present. */
	//TODO
	//Also verify if GenuineIntel is returned in vendor id string.
	vendor_id = (char*)(GetFromCpuId(VENDOR_ID));
	if ( !vendor_id || strcmp(vendor_id, "GenuineIntel") )
		return -1;
	return 0;
}

