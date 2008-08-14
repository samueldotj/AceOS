/*! 
	\file 	ds/sort.h
	\brief 	Array Sort
*/

#ifndef SORT__H
#define SORT__H

#include <ace.h>

#ifdef __cplusplus
    extern "C" {
#endif

void SortArray(char * data_array, char * temp_data, int data_size, int total_elements, COMPARISION_RESULT (*compare_data)(char * data1, char * data2) );

#ifdef __cplusplus
	}
#endif


#endif


