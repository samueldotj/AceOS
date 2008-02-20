/*!
	\file		spinlock.c
	\author		Samuel(samueldotj@gmail.com)
	\version 	3.0
	\date	
				Created: 21/01/08
				Last modified: 21/01/08
	\brief		spinlock implementation - architecture independ
*/

#ifndef SPINLOCK__H
#define SPINLOCK__H

#define SPIN_LOCK_TRY_COUNT 5000000

#define BIT_LOCK_SUCCESS 0
#define BIT_LOCK_FAILURE 1

typedef struct spinlock
{
	void * data;
}SPIN_LOCK, * SPIN_LOCK_PTR;

#ifdef __cplusplus
    extern "C" {
#endif

__inline__ void InitSpinLock(SPIN_LOCK_PTR pLockData);
__inline__ void SpinLock(SPIN_LOCK_PTR pLockData);
__inline__ void SpinUnlock(SPIN_LOCK_PTR pLockData);

void BitSpinLock(void * pLockData, int iPos);
void BitSpinUnlock(void * pLockData, int iPos);
int BitSpinLockTry(void * pLockData, int iPos);


#ifdef __cplusplus
	}
#endif

#endif
