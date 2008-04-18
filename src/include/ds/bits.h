/*!
  \file	src/include/bits.h
  \author	Samuel
  \version 	3.0
  \date	
  			Created: 3:29 PM 07-Apr-08
  			Last modified: Wed Apr 09, 2008  01:33AM
  \brief	Contains bit manipulation macros.
*/

#ifndef _BITS_H_
#define _BITS_H_

#include <ace.h>

inline int GetBitFromBitArray(void * bit_array, UINT32 bit_index);
inline void SetBitInBitArray(void * bit_array, UINT32 bit_index);
inline void ClearBitInBitArray(void * bit_array, UINT32 bit_index);
inline UINT32 FindFirstSetBitInBitArray(void * bit_array, UINT32 length, UINT32 * result);
inline UINT32 FindFirstClearBitInBitArray(void * bit_array, UINT32 length, UINT32 * result);
	   
#endif
