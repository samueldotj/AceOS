/*!
	\file		spinlock.c
	\brief		spinlock implementation - architecture independ header
*/

#ifndef SPINLOCK__H
#define SPINLOCK__H

/*! if the lock is busy how many times to spin before panic*/
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

inline void InitSpinLock(SPIN_LOCK_PTR pLockData);
inline int SpinLock(SPIN_LOCK_PTR pLockData);
inline void SpinUnlock(SPIN_LOCK_PTR pLockData);

inline int BitSpinLock(void * pLockData, int iPos);
inline void BitSpinUnlock(void * pLockData, int iPos);
inline int BitSpinLockTry(void * pLockData, int iPos);


#ifdef __cplusplus
	}
#endif

#endif
