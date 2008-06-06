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
enum ERROR_CODE
{
	ERROR_SUCCESS = 0,
	ERROR_NOT_ENOUGH_MEMORY,
	ERROR_NOT_READY,
	ERROR_WRITE_FAULT,
	ERROR_READ_FAULT,
	ERROR_GEN_FAILURE,
	ERROR_LOCK_VIOLATION,
	ERROR_NOT_SUPPORTED,
	ERROR_INVALID_PARAMETER,
	ERROR_CALL_NOT_IMPLEMENTED,
	ERROR_BUSY,
	ERROR_IO_DEVICE,
	ERROR_IRQ_BUSY,
	ERROR_COUNTER_TIMEOUT,
	ERROR_RETRY,
	ERROR_TIMEOUT,
};

#endif
