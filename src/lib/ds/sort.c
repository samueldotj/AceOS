/*!
  \file		sort.c
  \brief	Array Sort using Heap sort - http://en.wikipedia.org/wiki/Heapsort
		http://www.iti.fh-flensburg.de/lang/algorithmen/sortieren/heap/heapen.htm
*/
#include <ace.h>
#include <ds/sort.h>
#include <string.h>

static void swap(char * data_array, int pos1, int pos2, char * temp, int data_size);
static void downheap(int v, char * data_array, int data_size, int total_elements, char * temp_data, COMPARISION_RESULT (*compare_data)(char * data1, char * data2));
static void buildheap(char * data_array, int data_size, int total_elements, char * temp_data, COMPARISION_RESULT (*compare_data)(char * data1, char * data2));

/*! Sorts a array - using heap sort algorithm
	\param data_array - array of data structure
	\param temp_data - temporary storage of same size as one data element
	\param data_size - size of a single data structure
	\param total_elements - total_elements in the array
*/
void SortArray(char * data_array, char * temp_data, int data_size, int total_elements, COMPARISION_RESULT (*compare_data)(char * data1, char * data2))
{
	buildheap(data_array, data_size, total_elements, temp_data, compare_data);
	while (total_elements>1)
	{
		total_elements--;
		swap( data_array, 0, total_elements, temp_data, data_size );
		downheap (0, data_array, data_size, total_elements, temp_data, compare_data);
	} 
}

static void buildheap(char * data_array, int data_size, int total_elements, char * temp_data, COMPARISION_RESULT (*compare_data)(char * data1, char * data2))
{
	int v;
	for (v=total_elements/2-1; v>=0; v--)
		downheap (v, data_array, data_size, total_elements, temp_data, compare_data);
}

static void downheap(int v, char * data_array, int data_size, int total_elements, char * temp_data, COMPARISION_RESULT (*compare_data)(char * data1, char * data2))
{
	int w=2*v+1;    // first descendant of v
	while (w<total_elements)
	{
		if (w+1<total_elements)    // is there a second descendant?
		{
			if( compare_data( &data_array[ (w+1)*data_size ] , &data_array[ w * data_size ] ) == GREATER_THAN )
				w++;
		}
		
		// w is the descendant of v with maximum label
		if( compare_data( &data_array[ (v)*data_size ] , &data_array[ w * data_size ] ) != LESS_THAN )
			return; // v has heap property
		
		// otherwise
		swap( data_array, v, w, temp_data, data_size );
		v=w;        // continue
		w=2*v+1;
	}
}
static void swap(char * data_array, int pos1, int pos2, char * temp, int data_size)
{
	memmove( temp, &data_array[pos1*data_size], data_size );
	memmove( &data_array[pos1*data_size], &data_array[pos2*data_size], data_size );
	memmove( &data_array[pos2*data_size], temp, data_size );
}
