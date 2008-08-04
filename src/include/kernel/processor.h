/*!
  \file		kernel/processor.h
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created:
  			Last modified: Mon Aug 04, 2008  04:24PM
  \brief	
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
