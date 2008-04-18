/*!
	\file		bits.c
	\author		Samuel
	\version 	1.0
	\date	
  			Created: 18-Apr-2008 15:34
  			Last modified: 18-Apr-2008 15:34
	\brief	bit manipulation

*/

#include <ds/bits.h>

/*!	returns the value of bit (0 or 1) at the bit_index in the bit_array
	\param 	bit_array - starting address of the bit_array
	\param	bit_index - index of the bit
	\return	value of the bit (0 or 1)
*/
inline int GetBitFromBitArray(void * bit_array, UINT32 bit_index)
{
	return ((((BYTE*)bit_array)[bit_index / BITS_PER_BYTE]) >> (bit_index % BITS_PER_BYTE) ) & 1;
}

/*!	sets bit value to 1 at the bit_index in the bit_array
	\param 	bit_array - starting address of the bit_array
	\param 	bit_index - index of the bit
	\return void
*/
inline void SetBitInBitArray(void * bit_array, UINT32 bit_index)
{
	
	((BYTE*)bit_array)[bit_index / BITS_PER_BYTE] |= (1<< (bit_index % BITS_PER_BYTE) );
}

/*!	resets bit value to 0 at the bit_index in the bit_array
	\param 	bit_array - starting address of the bit_array
	\param	bit_index - index of the bit
	\return	void
*/
inline void ClearBitInBitArray(void * bit_array, UINT32 bit_index)
{
	((BYTE*)bit_array)[bit_index / BITS_PER_BYTE] &= (~(1<< (bit_index % BITS_PER_BYTE) ) );
}

/*!	finds the first set bit in the array
	\param 	bit_array - starting address of the bit_array
	\param	length - total bits in the bit array
	\return	0 if success else -1
*/
inline UINT32 FindFirstSetBitInBitArray(void * bit_array, UINT32 length, UINT32 * result)
{
	UINT32 i, length_in_byte = length/BITS_PER_BYTE + 1;
	for(i=0; i<length_in_byte; i++)
	{
		register BYTE byte = ((BYTE*)bit_array)[i];
		if ( byte != 0 )
		{
			int j;
			for(j=0; j<BITS_PER_BYTE; j++ )
			{
				if ( byte  & (1<<j) )
				{
					*result = (i * BITS_PER_BYTE) + j;
					return 0;
				}
			}
		}
	}
	return -1;
}
/*!	finds the first cleared bit in the array
	\param 	bit_array - starting address of the bit_array
	\param	length - total bits in the bit array
	\return	0 if success else -1
*/
inline UINT32 FindFirstClearBitInBitArray(void * bit_array, UINT32 length, UINT32 * result)
{
	UINT32 i, length_in_byte = length/BITS_PER_BYTE + 1;
	for(i=0;i<length_in_byte;i++)
	{
		register BYTE byte = ((BYTE*)bit_array)[i];
		if ( byte != 0xFF )
		{
			int j;
			for(j=0; j<BITS_PER_BYTE; j++ )
			{
				if ( !(byte  & (1<<j)) )
				{
					*result = (i * BITS_PER_BYTE) + j;
					return 0;
				}
			}
		}
	}
	return -1;
}
