/*!
	\file		atomic.c
	\author		Samuel(samueldotj@gmail.com)
	\version 	3.0
	\date	
				Created: 18-Mar-08
				Last modified: 
	\brief		Atomic Operations
	This file contains neccessary routines to do atomic operations(add, sub) on a integer
	Note :	This code is i386 specific
*/
#include <ace.h>
#include <kernel/atomic.h>

/*!AtomicAdd - Add integer to atomic variable
	i - integer value to add
	v - pointer of type ATOMIC
	Atomically adds @i to @v.
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
/*!AtomicSub - Subtract the atomic variable

	i - Integer value to subtract
	v - Pointer of type atomic_t
	Atomically subtracts @i from @v. 
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
