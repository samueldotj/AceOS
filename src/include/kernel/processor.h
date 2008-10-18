/*!
  \file		kernel/processor.h
  \brief	Processor data structure - architecture independent part

		Architecture depended and independed processor structures are maintained using parallel arrays.
		Architecture depended processor structure is declared inside arch/processor.h
*/

#ifndef _PROCESSOR_H_
#define _PROCESSOR_H_

#include <ace.h>

typedef struct processor *PROCESSOR_PTR;
#include <kernel/pm/task.h>
#include <kernel/pm/scheduler.h>

#ifdef CONFIG_SMP
	#define MAX_PROCESSORS	64		/*! Maximum processors supported by Ace*/
#else
	#define MAX_PROCESSORS	1
#endif

#define GET_CURRENT_PROCESSOR	(PROCESSOR_PTR)(GetCurrentThread()->current_processor)

enum PROCESSOR_STATE
{
	PROCESSOR_STATE_ONLINE,
	PROCESSOR_STATE_OFFLINE
};

/*! Data structure for architecture independed part of a processor*/
typedef struct processor
{
	SPIN_LOCK			lock;
	BOOLEAN 			state;					/*! state of the processor */
	
	THREAD_PTR			running_thread;			/*! pointer to currently running thread on this processor */
	
	READY_QUEUE_PTR 	active_ready_queue;		/*! pointer to active ready queue on this processor */
	READY_QUEUE_PTR 	dormant_ready_queue;	/*! pointer to dormant ready queue on this processor */
	
	char				loaded;					/*! indicates if this processor is heavily loaded(1) or not(0). */
	
	THREAD_PTR			idle_thread;			/*! idle thread for this processor */
}PROCESSOR;


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

void InitProcessors();


#endif
