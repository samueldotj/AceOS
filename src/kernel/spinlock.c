/*!
  \file		kernel/spinlock.c
  \brief	Contains kernel specific implementation of spinlock
		
			This file exports functions which is required by the sync library
*/
#include <sync/spinlock.h>
#include <kernel/debug.h>
#include <kernel/pm/elf.h>
void SpinLockTimeout(SPIN_LOCK_PTR pLockData, void * caller)
{
	char * func, * data;
	int func_offset, data_offset;
	
	func = FindKernelSymbolByAddress( (VADDR) pLockData->last_locker, &func_offset);
	data = FindKernelSymbolByAddress( (VADDR) pLockData, &data_offset);
	kprintf("\nSpinlock Timeout - %s+%d:%p \nLocked by %s+%d:%p\n", data, data_offset, pLockData, func, func_offset, pLockData->last_locker);
	panic("Lock timeout");
}
