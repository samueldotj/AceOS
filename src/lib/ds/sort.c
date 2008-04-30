/*!
  \file		sort.c
  \author	Samuel
  \version 	1.0
  \date	
  			Created: 28-Apr-2008 18:18
  			Last modified: 28-Apr-2008 18:29
  \brief	Array Sort using Heap sort - http://en.wikipedia.org/wiki/Heapsort
*/
#include <ace.h>
#include <ds/sort.h>
#include <string.h>

static void sift_down(char * data_array, char * temp_data, int root, int bottom, int data_size, COMPARISION_RESULT (*compare_data)(char * data1, char * data2) );
static void swap(char * data_array, int pos1, int pos2, char * temp, int data_size);

/*! Sorts a array - using heap sort algorithm
	\param data_array - array of data structure
	\param temp_data - temporary storage of same size as one data element
	\param data_size - size of a single data structure
	\param total_elements - total_elements in the array
*/
void SortArray(char * data_array, char * temp_data, int data_size, int total_elements, COMPARISION_RESULT (*compare_data)(char * data1, char * data2) )
{
	int i;
	
	for (i = (total_elements / 2)-1; i >= 0; i--)
		sift_down(data_array, temp_data, i, total_elements, data_size, compare_data);

	for (i = total_elements-1; i >= 1; i--)
	{
		swap( data_array, 0, i, temp_data, data_size );
		sift_down(data_array, temp_data, 0, i-1, data_size, compare_data);
	}
}
static void sift_down(char * data_array, char * temp_data, int root, int bottom, int data_size, COMPARISION_RESULT (*compare_data)(char * data1, char * data2) )
{
	int done, maxChild;

	done = 0;
	while ( (root*2) <= bottom && !done )
	{
		if ( (root*2) == bottom)
			maxChild = root * 2;
		else if( compare_data( &data_array[ (root*2)*data_size ] , &data_array[ (root*2+1) * data_size ] ) == GREATER_THAN )
			maxChild = root * 2;
		else
			maxChild = root * 2 + 1;

		if( compare_data( &data_array[root * data_size], &data_array[maxChild * data_size] ) == LESS_THAN )
		{
			swap( data_array, root, maxChild, temp_data, data_size);
			root = maxChild;
		}
		else
			done = 1;
	}
}

static void swap(char * data_array, int pos1, int pos2, char * temp, int data_size)
{
	memcpy( temp, &data_array[pos1*data_size], data_size );
	memcpy( &data_array[pos1*data_size], &data_array[pos2*data_size], data_size );
	memcpy( &data_array[pos2*data_size], temp, data_size );
}
