/*!
  \file		kernel/processor.c
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created:
  			Last modified: Mon Aug 04, 2008  04:00PM
  \brief	
*/

#include <ace.h>
#include <kernel/processor.h>

PROCESSOR processor[MAX_PROCESSORS];
volatile int count_running_processors;
