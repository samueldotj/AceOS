/*!
  \file		kernel/spinlock.c
  \brief	Contains kernel specific implementation of spinlock
		
			This file exports functions which is required by the sync library
*/
#include <kernel/debug.h>
#include <sync/spinlock.h>
void SpinLockTimeout(SPIN_LOCK_PTR pLockData, void * caller)
{
	kprintf("Spinlock Timeout at %p [Data %p] Locked by %p %d\n", caller, pLockData, pLockData->last_locker, pLockData->locked);
	panic("Lock timeout");
}
