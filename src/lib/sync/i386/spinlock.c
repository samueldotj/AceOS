/*!
	\file		spinlock.c
	\brief		SpinLock for Ace
	Note - This spinlock code is specific to i386
*/
#include <ace.h>
#include <assert.h>
#include <stdlib.h>
#include <sync/spinlock.h>

/*!	InitSpinLock - initialize the spinlock data structure
	\param pSpinLock - Pointer to the spinlock structure
*/
inline void InitSpinLock(SPIN_LOCK_PTR pSpinLock)
{
	pSpinLock->last_locker = __builtin_return_address(0);
	pSpinLock->locked = 0;
}

/*!	SpinLock - Spin to get a lock until timeout occurs
	\param pSpinLock - Pointer to the spinlock structure
	\return 0 on success and non-zero on timeout.
*/
inline int SpinLock(SPIN_LOCK_PTR pSpinLock)
{
	UINT32 result=0, count=SPIN_LOCK_TRY_COUNT;
	/*loop until acquiring the lock*/
	do
	{
		asm volatile("\
			movl $1, %%eax;\
        	lock xchgl %%eax, (%%edx);\
			pause"
            :"=a"(result)
            :"d"(&pSpinLock->locked)
			);
	}while( result && count-- );
	if ( result )
		SpinLockTimeout(pSpinLock, __builtin_return_address(0) );
	
	pSpinLock->last_locker = __builtin_return_address(0);
	
	return result;
}

/*! 	TrySpinLock - Actually there is no spin here - it just tries to acquire the lock only once;
	\param pSpinLock - Pointer to the spinlock structure
	\return 0 on sucess and non-zero on failure 
*/
inline int TrySpinLock(SPIN_LOCK_PTR pSpinLock)
{
	int result=1;
	asm volatile("\
			movl $1, %%eax;\
       	    lock xchgl %%eax, (%%edx);"
           	:"=a"(result)
            :"d"(&pSpinLock->locked)
			);
	/*if locked update the caller address */
	if ( result == 0 )
		pSpinLock->last_locker = __builtin_return_address(0);
	return result;
}

/*! 	SpinUnlock - unlocks the given spinlock data
	\param pSpinLock - Pointer to the spinlock structure
*/
inline void SpinUnlock(SPIN_LOCK_PTR pSpinLock)
{
	int result;

	/*adding lock prefix causes an invalid instruction execption
	although lock is not neccesssary for an unlock operation - it is good to have*/
	asm volatile("\
		movl $0, %%eax;\
		lock xchgl %%eax, (%%edx);"
        : "=a" (result)
        :"d"(&pSpinLock->locked)
		);

	assert( result == 1 );
}
/*! 	BitSpinLock - Spin to get a bit lock until timeout occurs
	\param pBitData - Pointer to the bit lock array
	\param iPos - Bit Position
	\return 0 on sucess and non-zero on timeout 
*/
inline int BitSpinLock(void * pBitData, int iPos)
{
	UINT32 i, count=SPIN_LOCK_TRY_COUNT;
	/*loop until acquiring the lock*/
	do
	{
		i = 0;
		asm volatile("\
			lock bts %%ecx, (%%edx);\
			mov $1, %%ebx;\
			cmovcl %2, %%ebx;\
			pause"
			:
			:"c"(iPos), "d"(pBitData), "m"(i)
		);
	}while( i && count-- );

	return i;
}

/*! 	BitSpinLockTry - Actually there is no spin here - it just tries to acquire the  lock only once; 
	\param pBitData - Pointer to the bit lock array
	\param iPos - Bit Position
	\return 0 on sucess and non-zero on failure 
*/
inline int BitSpinLockTry(void * pBitData, int iPos)
{
	int result;
	asm volatile("\
		lock bts %%ecx, (%%edx);\
		mov $1, %%ebx;\
		cmovcl %%eax, %%ebx;"		/*store 1 in eax if locking is failed (ebx contains 1)*/
		:"=a"(result)
		:"c"(iPos), "d"(pBitData)	/*load the values into ecx and edx*/
		
		);
	return result;
}
/*! 	BitSpinUnlock - unlocks the given spinlock bit
	\param pBitData - Pointer to the bit lock array
	\param iPos - Bit Position
*/
inline void BitSpinUnlock(void * pBitData, int iPos)
{
	UINT32 data = *(UINT32*)pBitData;
	assert ( ((UINT32)data & (1<<iPos)) == 1 );

	asm volatile("lock btr %%ecx, (%%edx);"
		:
		:"c"(iPos), "d"(pBitData)
		);
}
