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

static CPUID_INFO cpuid_info;

#define EXEC_CPUID_INSTRUCTION	__asm__ volatile("mov %4, %%eax; CPUID; mov %%eax, %0; mov %%ebx, %1; mov %%ecx, %2; mov %%edx, %3" \
					:"=m"(reg_eax), "=m"(reg_ebx), "=m"(reg_ecx), "=m"(reg_edx) \
					:"m"(input) \
					);

void GetAttributesFromEax0(void)
{
	UINT32 reg_eax, reg_ebx, reg_ecx, reg_edx, input;

	/* Load EAX with 0 and call CPUID instruction */
	input=0;
	//EXEC_CPUID_INSTRUCTION();
	__asm__ volatile("mov %4, %%eax; CPUID; mov %%eax, %0; mov %%ebx, %1; mov %%ecx, %2; mov %%edx, %3" 
					:"=m"(reg_eax), "=m"(reg_ebx), "=m"(reg_ecx), "=m"(reg_edx)
                    :"m"(input)
                    );
	/* Output in EAX */
	cpuid_info.eax_max = reg_eax;
	
	/* Output in EBX */
	cpuid_info.vendor_id[0] = EXTRACT_BITS(reg_ebx, 0, 7);
	cpuid_info.vendor_id[1] = EXTRACT_BITS(reg_ebx, 8, 15);
	cpuid_info.vendor_id[2] = EXTRACT_BITS(reg_ebx, 16, 23);
	cpuid_info.vendor_id[3] = EXTRACT_BITS(reg_ebx, 24, 31);

	/* Output in ECX */
	cpuid_info.vendor_id[4] = EXTRACT_BITS(reg_ecx, 0, 7);
	cpuid_info.vendor_id[5] = EXTRACT_BITS(reg_ecx, 8, 15);
	cpuid_info.vendor_id[6] = EXTRACT_BITS(reg_ecx, 16, 23);
	cpuid_info.vendor_id[7] = EXTRACT_BITS(reg_ecx, 24, 31);
	
	/* Output in EDX */
	cpuid_info.vendor_id[8] = EXTRACT_BITS(reg_edx, 0, 7);
	cpuid_info.vendor_id[9] = EXTRACT_BITS(reg_edx, 8, 15);
	cpuid_info.vendor_id[10] = EXTRACT_BITS(reg_edx, 16, 23);
	cpuid_info.vendor_id[11] = EXTRACT_BITS(reg_edx, 24, 31);
	cpuid_info.vendor_id[12] = '\0';
	return;
}


void GetAttributesFromEax1(void)
{
	UINT32 reg_eax, reg_ebx, reg_ecx, reg_edx, input;

	/* Load EAX with 1 and call CPUID instruction */
	input=1;
	//EXEC_CPUID_INSTRUCTION();
	__asm__ volatile("mov %4, %%eax; CPUID; mov %%eax, %0; mov %%ebx, %1; mov %%ecx, %2; mov %%edx, %3" 
					:"=m"(reg_eax), "=m"(reg_ebx), "=m"(reg_ecx), "=m"(reg_edx)
                    :"m"(input)
                    );
	/* Output in EAX */
	cpuid_info.stepping_id = EXTRACT_BITS(reg_eax, 0, 3);
	cpuid_info.model_number = EXTRACT_BITS(reg_eax, 4, 7);
	cpuid_info.family_code = EXTRACT_BITS(reg_eax, 8, 11);
	cpuid_info.processor_type = EXTRACT_BITS(reg_eax, 12, 13);
	cpuid_info.extended_model = EXTRACT_BITS(reg_eax, 16, 19);
	cpuid_info.extended_family = EXTRACT_BITS(reg_eax, 20, 27);
	
	/* Output in EBX */
	cpuid_info.brand_id = EXTRACT_BITS(reg_ebx, 0, 7);
	cpuid_info.cache_line_size = EXTRACT_BITS(reg_ebx, 8, 15); //this is same as chunks
	cpuid_info.count_logical_processors = EXTRACT_BITS(reg_ebx, 16, 23);
	cpuid_info.apic_id = EXTRACT_BITS(reg_ebx, 24, 31);

	/* Output in ECX */
	cpuid_info.feature_flags_ecx.feature_flags = reg_ecx;
	/* Output in EDX */
	cpuid_info.feature_flags_edx.feature_flags = reg_edx;
}

void GetAttributesFromEax2(void)
{
	UINT32 reg_eax, reg_ebx, reg_ecx, reg_edx, input;

	/* Load EAX with 2 and call CPUID instruction */
	input=2;
	//EXEC_CPUID_INSTRUCTION();
	__asm__ volatile("mov %4, %%eax; CPUID; mov %%eax, %0; mov %%ebx, %1; mov %%ecx, %2; mov %%edx, %3" 
					:"=m"(reg_eax), "=m"(reg_ebx), "=m"(reg_ecx), "=m"(reg_edx)
                    :"m"(input)
                    );
	/* Output in EAX */
	
	/* Output in EBX */
	/* Output in ECX */
	/* Output in EDX */
}

void GetAttributesFromEax3(void)
{
	UINT32 reg_eax, reg_ebx, reg_ecx, reg_edx, input;

	/* Load EAX with 3 and call CPUID instruction */
	input=3;
	//EXEC_CPUID_INSTRUCTION();
	__asm__ volatile("mov %4, %%eax; CPUID; mov %%eax, %0; mov %%ebx, %1; mov %%ecx, %2; mov %%edx, %3" 
					:"=m"(reg_eax), "=m"(reg_ebx), "=m"(reg_ecx), "=m"(reg_edx)
                    :"m"(input)
                    );
	/* Output in EAX */
	
	/* Output in EBX */
	/* Output in ECX */
	/* Output in EDX */
}

/*!
	\brief	Loads the cpuid_info structure with all available cpuid information. 

	\param	 

	\return	 void
*/
void LoadDataStructureFromCpuID()
{
	/* call CPUID with all possible values and store all the required information */
	/* After this call, we can just read value from data structures without execting CPUID instruction every time */

	/* EAX=0 */
	GetAttributesFromEax0();

	/* EAX=1 */
	GetAttributesFromEax1();

	/* EAX=2 */
	GetAttributesFromEax2();

	/* EAX=3 */
	GetAttributesFromEax3();
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
		case APIC_ID					: return &(cpuid_info.apic_id);
		//TODO.. feature flags for eax=1
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



void GetFeatureFlagsEax1(UINT32 reg_ecx, UINT32 reg_edx)
{
}
