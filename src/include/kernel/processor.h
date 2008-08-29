/*!
  \file		kernel/processor.h
  \brief	Processor data structure - architecture independent part

		Architecture depended and independed processor structures are maintained using parallel arrays.
		Architecture depended processor structure is declared inside arch/processor.h
*/

#ifndef _PROCESSOR_H_
#define _PROCESSOR_H_

#include <ace.h>

#ifdef CONFIG_SMP
	#define MAX_PROCESSORS	64		/*! Maximum processors supported by Ace*/
#else
	#define MAX_PROCESSORS	1
#endif


enum PROCESSOR_STATE
{
	PROCESSOR_STATE_ONLINE,
	PROCESSOR_STATE_OFFLINE
};

/*! Data structure for architecture independed part of a processor*/
typedef struct processor
{
	BOOLEAN 	state;	/*!state of the processor*/
}PROCESSOR, *PROCESSOR_PTR;

/*! All processors in the system - Processors are indexed by using APIC ID
	so there might be hole in this structure which are zero filled.
*/
extern PROCESSOR processor[MAX_PROCESSORS];
extern volatile int count_running_processors;

/*! Get current processor's unique id*/
UINT16 GetCurrentProcessorId();

extern UINT16 master_processor_id;

/*! Returns curren processor's LAPIC's base address*/
void * GetLAPICBaseAddress();

#endif
