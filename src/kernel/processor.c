/*!
  \file		kernel/processor.c
  \brief	
*/

#include <ace.h>
#include <kernel/processor.h>

PROCESSOR processor[MAX_PROCESSORS];
volatile int count_running_processors;
