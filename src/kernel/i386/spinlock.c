/*!
	\file		spinlock.c
	\author		Samuel(samueldotj@gmail.com)
	\version 	3.0
	\date	
				Created: 21/01/08
				Last modified: 21/01/08
	\brief		SpinLock for Ace
	Note - This spinlock code is specific to i386
*/
#include <ace.h>
#include <kernel/debug.h>
#include <kernel/arch.h>
#include <kernel/spinlock.h>

/*! InitSpinLock - initialize the spinlock data structure
*/
inline void InitSpinLock(SPIN_LOCK_PTR pSpinLock)
{
	pSpinLock->data = 0;
}

/*! SpinLock - Spin to get a lock until timeout occurs
*/
inline void SpinLock(SPIN_LOCK_PTR pSpinLock)
{
	UINT32 result, count=SPIN_LOCK_TRY_COUNT;
	/*loop until acquiring the lock*/
	do
	{
		asm volatile("\
			movl $1, %%eax;\
        	lock xchgl %%eax, (%%edx);"
            :"=a"(result)
            :"d"(pSpinLock)
			);
	}while( result && count-- );
	/*check for timeout*/
	if ( result )
		panic("SpinLock timeout");
}

/*! TrySpinLock - Actually there is no spin here - it just tries to acquire the 
lock only once; returns success or failure.
*/
inline int TrySpinLock(SPIN_LOCK_PTR pSpinLock)
{
	asm volatile("\
			movl $1, %%eax;\
       	    lock xchgl %%eax, (%%edx);"
           	:
            :"d"(pSpinLock)
            :"%eax"
			);
	/*compiler will generate warning, however return value is already in EAX*/
}

/*! SpinUnlock - unlocks the given spinlock data
*/
inline void SpinUnlock(SPIN_LOCK_PTR pSpinLock)
{
	assert( pSpinLock->data != 0 );
	
	/*adding lock prefix causes an invalid instruction execption
	although lock is not neccesssary for an unlock operation - it is good to have*/
	asm volatile("movl $0, (%%ebx)"
        :
        :"b"(pSpinLock)
		);
}
/*! SpinLock - Spin to get a bit lock until timeout occurs
*/
void BitSpinLock(void * pSpinLock, int iPos)
{
	UINT32 i, count=SPIN_LOCK_TRY_COUNT;
	/*loop until acquiring the lock*/
	do
	{
		i = 0;
		asm volatile("\
			lock bts %%ecx, (%%edx);\
			mov $1, %%ebx;\
			cmovcl %2, %%ebx;"
			:
			:"c"(iPos), "d"(pSpinLock), "m"(i)
		);
	}while( i && count-- );
	if ( i )
		panic("SpinLock timeout\n");
}

/*! TrySpinLock - Actually there is no spin here - it just tries to acquire the 
lock only once; returns success or failure.
*/
int BitSpinLockTry(void * pSpinLock, int iPos)
{
	asm volatile("\
		lock bts %%ecx, (%%edx);\
		mov $1, %%ebx;\
		cmovcl %%eax, %%ebx;"		/*store 1 in eax if locking is failed (ebx contains 1)*/
		:
		:"c"(iPos), "d"(pSpinLock)	/*load the values into ecx and edx*/
		:"%eax"
		);
}
/*! SpinUnlock - unlocks the given spinlock bit
*/
void BitSpinUnlock(void * pSpinLock, int iPos)
{
	UINT32 data = *(UINT32*)pSpinLock;
	assert ( ((UINT32)data & (1<<iPos)) == 1 );

	asm volatile("\lock btr %%ecx, (%%edx);"
		:
		:"c"(iPos), "d"(pSpinLock)
		);
}
