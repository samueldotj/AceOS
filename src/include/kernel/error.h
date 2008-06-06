/*!
	\file		src/include/kernel/error.h	
	\author		Samuel
	\version 	3.0
	\date	
  			Created: 06-Jun-2008 12:04
  			Last modified: 06-Jun-2008 12:04
	\brief	Contains definition of all error values
*/

#ifndef __ERROR__H
#define __ERROR__H

#include <ace.h>
typedef enum
{
	ERROR_SUCCESS = 0,

	ERROR_NOT_SUPPORTED,
	ERROR_INVALID_PARAMETER,
	
	ERROR_NOT_ENOUGH_MEMORY,
	
	ERROR_WRITE_FAULT,
	ERROR_READ_FAULT,
	
	ERROR_LOCK_VIOLATION,
		
	ERROR_BUSY,
	ERROR_RETRY,
	ERROR_TIMEOUT,
	
	ERROR_IO_DEVICE,
	ERROR_IRQ_BUSY,
}ERROR_CODE;

#endif
