/*!
  \file		kernel/processor.h
  \brief	Processor
*/


#ifndef _PROCESSOR_H_
#define _PROCESSOR_H_

enum PROCESSOR_STATE
{
	PROCESSOR_STATE_ONLINE,
	PROCESSOR_STATE_OFFLINE
};

typedef struct processor
{
	UINT32 apic_id;
	BOOLEAN state;
}PROCESSOR, *PROCESSOR_PTR;

extern PROCESSOR processor[MAX_PROCESSORS];
extern volatile int count_running_processors;

#endif
