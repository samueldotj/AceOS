/*!
  \file		ds/bits.h
  \brief	Contains bit manipulation macros.
*/

#ifndef _BITS_H_
#define _BITS_H_

#include <ace.h>

/* Extracts bit value in src from xth bit to yth bit inclusive.
 * x and y are measured from lsb and x < y. x=1 means 1st bit.
 */
#define EXTRACT_BITS(src, x, y) \
    (((unsigned long int)src<<((sizeof(unsigned long int)*BITS_PER_BYTE)-(y)))>>((sizeof(unsigned long int)*BITS_PER_BYTE)-(y)+(x)-1))

/*! Swaps the value of x and y of type "type"*/
#define SWAP( x, y, type ) 	\
{							\
	type tmp;				\
	tmp = x;				\
	x = y;					\
	y = tmp;				\
}

/*! Evaluvates whether the given "value" is with in the given range */
#define VALUE_WITH_IN_RANGE( range_start, range_end, value )	( (value) >= (range_start) && value <= (range_end) )
/*! Evaluvates whether the given start and end is with in the given range */
#define RANGE_WITH_IN_RANGE( range_start, range_end, start, end )	\
					( VALUE_WITH_IN_RANGE(range_start, range_end, start) && VALUE_WITH_IN_RANGE(range_start, range_end, end) )

inline int FindFirstSetBitInLong(register unsigned long value);

inline int GetBitFromBitArray(void * bit_array, UINT32 bit_index);
inline void SetBitInBitArray(void * bit_array, UINT32 bit_index);
inline void ClearBitInBitArray(void * bit_array, UINT32 bit_index);
inline long FindFirstSetBitInBitArray(void * bit_array, UINT32 length);
inline UINT32 FindFirstClearBitInBitArray(void * bit_array, UINT32 length, UINT32 * result);
	   
#endif
