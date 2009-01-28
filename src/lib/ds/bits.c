/*!
	\file		bits.c
	\brief	bit manipulation

*/

#include <ds/bits.h>
#include <assert.h>
#include <stdlib.h>
/*! Finds the first set bit in a long
	\param value - long to scan
	\return On success
	 *			0-31 on 32 bit architecture
	 *			0-63 on 64 bit architecture
	 *		On failure
	 *			-1 
 */
inline int FindFirstSetBitInLong(register unsigned long value)
{
	register unsigned int result=0;
	register unsigned int shift;
	
	if ( value == 0 )
		return -1;

#if ARCH == x86_64 
	shift = (value > 0xFFFFFFFF) << 5; value >>= shift; result |= shift;
#endif
	shift = (value > 0xFFFF) << 4; value >>= shift; result |= shift;
	shift = (value > 0xFF  ) << 3; value >>= shift; result |= shift;
	shift = (value > 0xF   ) << 2; value >>= shift; result |= shift;
	shift = (value > 0x3   ) << 1; value >>= shift; result |= shift;
	
	result |= (value >> 1);

	return result;
}
/*!	finds the first set bit in the array
	\param 	bit_array - starting address of the bit_array
	\param	length - total bits in the bit array
	\return	0-63 if success else -1
*/
inline long FindFirstSetBitInBitArray(void * bit_array, UINT32 length)
{
	register UINT32 i=0 , length_in_long=0;
	
	/*bit array pointer should not be null*/
	assert( bit_array != NULL );
	
	/*bit array size should be multiple of long*/
	assert( length % (sizeof(unsigned long)*BITS_PER_BYTE) == 0 );
	
#if ARCH == i386
	length_in_long = length >> 5;		/*for 32 bit architectures divide by 32 to get number of longs in the input to scan*/
#elif ARCH == x86_64 
	length_in_long = length >> 6;		/*divide by 64*/
#else
	#error "Arch not defined"
#endif

	do 
	{
		register unsigned long value = ((unsigned long *)bit_array)[i];
		if ( value )
		{
#if ARCH == i386
			return (i<<5) + FindFirstSetBitInLong( value );
#elif ARCH == x86_64 
			return (i<<6) + FindFirstSetBitInLong( value );
#endif
		}
		i++;
	}while(length_in_long<i);
	
	return -1;
}

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

/*!	finds the first cleared bit in the array
	\param 	bit_array - starting address of the bit_array
	\param	length - total bits in the bit array
	\param	result	Pointer to first clear bit in bit_array.
	\return	0 if success else -1
*/
inline UINT32 FindFirstClearBitInBitArray(void * bit_array, UINT32 length, UINT32 * result)
{
	UINT32 i, length_in_byte = length/BITS_PER_BYTE + 1;

	/* Only for buffer counts exactly equal to BITS_PER_BYTE, special care must be taken */
	if ( (length % BITS_PER_BYTE) == 0 )
	{
		length_in_byte--;
	}

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
