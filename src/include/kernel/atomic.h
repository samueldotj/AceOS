/*!
	\file		atomic.h
	\author		Samuel(samueldotj@gmail.com)
	\version 	3.0
	\date	
				Created: 18-Mar-08
				Last modified: 
	\brief		Atomic Operations
	This file contains neccessary routines to do atomic operations(add, sub) on a integer
	Note :	This code is i386 specific
*/

#ifndef __ATOMIC__H
#define __ATOMIC__H

#ifdef CONFIG_SMP
	#define LOCK "lock ; "
#else
	#define LOCK ""
#endif

/*
	Make sure gcc doesn’t try to be clever and move things around* on us. We need to use _exactly_ the address the user gave us,
	not some alias that contains the same information.
*/
typedef struct 
{
	volatile int data;
}ATOMIC;

#ifdef __cplusplus
	extern "C" {
#endif

static __inline__ void AtomicAdd(int i, ATOMIC *v);
static __inline__ void AtomicSub(int i, ATOMIC *v);

#ifdef __cplusplus
	}
#endif

#endif
