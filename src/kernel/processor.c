/*!
  \file		kernel/processor.c
  \brief	Architecture indepentend processor code
*/

#include <ace.h>
#include <kernel/processor.h>

PROCESSOR processor[MAX_PROCESSORS];
volatile int count_running_processors;

void InitProcessors()
{
	int i;
	for(i=0; i<MAX_PROCESSORS; i++)
	{
		InitSpinLock( &processor[i].lock );
		processor[i].state = (i==master_processor_id)? PROCESSOR_STATE_ONLINE:PROCESSOR_STATE_OFFLINE;
		processor[i].idle_thread = NULL;
		processor[i].running_thread = NULL;
	}
}
