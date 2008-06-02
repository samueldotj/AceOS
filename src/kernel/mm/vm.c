/*!
	\file		src/kernel/mm/vm.c	
	\author		Samuel
	\version 	3.0
	\date	
  			Created: 02-Jun-2008 10:34pm
  			Last modified: 02-Jun-2008 10:34pm
	\brief	vm related routines
*/
#include <kernel/mm/vm.h>
#include <kernel/mm/pmem.h>

/*! initializes the Virtual memory subsystem

*/
void InitVm()
{
	/*complete physical memory initialization*/
	InitPhysicalMemoryManagerPhaseII();
}
