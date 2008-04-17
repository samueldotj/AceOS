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

/*gets the bit value 1 or 0 of the specified index in a bit array - bit_index starts from 0*/
#define BIT_ARRAY_GET_BIT(bit_array, bit_index) \
	( ((((BYTE*)(bit_array))[(bit_index) / BITS_PER_BYTE]) >> ((bit_index) % BITS_PER_BYTE) ) & 1 )

/*sets the bit value to 1 in a bit array for a specified bit_index; bit_index starts from 0*/
#define BIT_ARRAY_SET_BIT(bit_array, bit_index) \
	(((BYTE*)(bit_array))[(bit_index) / BITS_PER_BYTE] |= (1<< ((bit_index) % BITS_PER_BYTE) ))

/*resets the bit value to 0 in a bit array for a specified bit_index; bit_index starts from 0*/
#define BIT_ARRAY_CLEAR_BIT(bit_array, bit_index) \
	((BYTE*)(bit_array))[(bit_index) / BITS_PER_BYTE] &= (~(1<< ((bit_index) % BITS_PER_BYTE) ) )

/* Finds the first set bit in a bit array
 * If Not found, returns -1
 */
#define BIT_ARRAY_FIND_FIRST_SET_BIT(bit_array, length_in_byte, result) 		\
	{																			\
		int i, found=FALSE;														\
		(result) = (length_in_byte);											\
		for(i=0; i<(length_in_byte) && !found;i++)								\
		{																		\
			register BYTE byte = ((BYTE*)(bit_array))[i];						\
			if ( byte != 0 )													\
			{																	\
				int j;															\
				for(j=0; j<BITS_PER_BYTE; j++ )									\
				{																\
					if ( byte  & (1<<j) )										\
					{															\
						(result) = (i * BITS_PER_BYTE) + j;						\
						found = TRUE;											\
						break;													\
					}															\
				}																\
			}																	\
		}																		\
		if ((result) == (length_in_byte))										\
		{																		\
			(result) = -1;														\
		}																		\
	}

/* Finds the first clear bit in a bit array
 * Returns -1 if not found
 */
#define BIT_ARRAY_FIND_FIRST_CLEAR_BIT(bit_array, length_in_byte, result) 		\
	{																			\
		int i, found = FALSE;													\
		(result) = (length_in_byte);											\
		for(i=0;i<(length_in_byte) && !found ;i++)								\
		{																		\
			register BYTE byte = ((BYTE*)(bit_array))[i];						\
			if ( byte != 0xFF )													\
			{																	\
				int j;															\
				for(j=0; j<BITS_PER_BYTE; j++ )									\
				{																\
					if ( !(byte  & (1<<j)) )									\
					{															\
						(result) = (i * BITS_PER_BYTE) + j;						\
						found = TRUE;											\
						break;													\
					}															\
				}																\
			}																	\
		}																		\
		if ((result) == (length_in_byte))										\
		{																		\
			(result) = -1;														\
		}																		\
	}
	   
#endif
