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
	VOID *	Data;
}SPIN_LOCK, * PSPIN_LOCK;

#ifdef __cplusplus
    extern "C" {
#endif

inline void InitSpinLock(PSPIN_LOCK_DATA pLockData);
inline void SpinLock(PSPIN_LOCK_DATA pLockData);
inline void SpinUnlock(PSPIN_LOCK_DATA pLockData);

void BitSpinLock(void * pLockData, int iPos);
void BitSpinUnlock(void * pLockData, int iPos);
int BitSpinLockTry(void * pLockData, int iPos);


#ifdef __cplusplus
	}
#endif

#endif
