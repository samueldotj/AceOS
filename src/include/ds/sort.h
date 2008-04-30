/*! 
	\file sort.h
	\brief Array Sort
	\author Samuel
	\date 
		Created: 28-Apr-2008 18:24
		Last modified: 28-Apr-2008 18:24
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


