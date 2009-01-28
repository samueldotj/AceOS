#include <ace.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <malloc.h>
#include <ds/bits.h>

void exit(int status);

void _assert(const char *msg, const char *file, int line)
{
	printf("assertion failed %s : %s %d", msg, file, line);
	exit(1);
}

inline int FindFirstSetBitInLong(register unsigned long value);

inline int GetBitFromBitArray(void * bit_array, UINT32 bit_index);
inline void SetBitInBitArray(void * bit_array, UINT32 bit_index);
inline void ClearBitInBitArray(void * bit_array, UINT32 bit_index);
inline long FindFirstSetBitInBitArray(void * bit_array, UINT32 length);
inline UINT32 FindFirstClearBitInBitArray(void * bit_array, UINT32 length, UINT32 * result);
	   
#define ARRAY_SIZE	10
int main(int argc, char* argv[])
{
	unsigned long value[ARRAY_SIZE];
	int i, return_value, bits_in_long = sizeof(unsigned long)*BITS_PER_BYTE;
	/*test FindFirstSetBitInLong*/
	for(i=0;i<bits_in_long;i++)
	{
		value[0] = 1<<i;
		return_value = FindFirstSetBitInLong(value[0]);
		if( return_value != i )
		{
			printf("FindFirstSetBitInLong(%ld) retured %d expected %d\n", value[0], return_value, i );
			exit(1);
		}
		if ( FindFirstSetBitInBitArray(value, bits_in_long) != return_value )
		{
			printf("FindFirstSetBitInBitArray() differs from FindFirstSetBitInLong()\n");
			exit(1);
		}
	}
	/*test GetBitFromBitArray, SetBitInBitArray and ClearBitInBitArray*/
	for(i=0;i<ARRAY_SIZE*bits_in_long;i++)
	{
		if ( i%2 )
			SetBitInBitArray( value, i );
		else
			ClearBitInBitArray( value, i );
	}
	for(i=ARRAY_SIZE*bits_in_long;i>0;i--)
	{
		return_value = GetBitFromBitArray(value, i);
		assert( return_value == 1 || return_value  == 0 );
		if ( (i%2 && return_value == 0) ||  ( !(i%2) && return_value == 1) )
		{
			printf("GetBitFromBitArray() retured %d expected %d\n", return_value, i%2 );
			exit(2);
		}
	}
	return 0;
}
