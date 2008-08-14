/*!
	\file		atomic.c
	\brief		Atomic Operations
	This file contains neccessary routines to do atomic operations(add, sub) on a integer
	Note :	This code is i386 specific
*/
#include <ace.h>
#include <kernel/atomic.h>

/*! Add integer to atomic variable - Atomically adds \a i to \a v.
 * \param i - integer value to add
 * \param v - pointer of type ATOMIC
 */
static __inline__ void AtomicAdd(int i, ATOMIC *v)
{
    __asm__ __volatile__
        ( 
        LOCK "addl %1,%0"
        :"=m" (v->data)
        :"ir" (i), "m" (v->data)
        );
}
/*! Subtract the atomic variable
 * \param i - Integer value to subtract
 * \param v - Pointer of type atomic_t
 */
static __inline__ void AtomicSub(int i, ATOMIC *v)
{
    __asm__ __volatile__
        (
        LOCK "subl %1,%0"
        :"=m" (v->data)
        :"ir" (i), "m" (v->data)
        );
}
