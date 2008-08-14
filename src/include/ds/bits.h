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

inline int GetBitFromBitArray(void * bit_array, UINT32 bit_index);
inline void SetBitInBitArray(void * bit_array, UINT32 bit_index);
inline void ClearBitInBitArray(void * bit_array, UINT32 bit_index);
inline UINT32 FindFirstSetBitInBitArray(void * bit_array, UINT32 length, UINT32 * result);
inline UINT32 FindFirstClearBitInBitArray(void * bit_array, UINT32 length, UINT32 * result);
	   
#endif
