/*!
	\file	kernel/error.h	
	\brief	Contains definition of all error values
*/

#ifndef __ERROR__H
#define __ERROR__H

#include <ace.h>
#include <enum.h>

#define ENUM_LIST_ERROR_CODE(_) \
	_(ERROR_SUCCESS,) \
	_(ERROR_NOT_SUPPORTED,) \
	_(ERROR_INVALID_PARAMETER,) \
	_(ERROR_NOT_ENOUGH_MEMORY,) \
	_(ERROR_WRITE_FAULT,) \
	_(ERROR_READ_FAULT,) \
	_(ERROR_LOCK_VIOLATION,) \
	_(ERROR_BUSY,) \
	_(ERROR_RETRY,) \
	_(ERROR_TIMEOUT,) \
	_(ERROR_IO_DEVICE,) \
	_(ERROR_IRQ_BUSY,) \
	_(ERROR_INVALID_FORMAT,) \
	_(ERROR_INVALID_PATH,) \
	_(ERROR_NOT_FOUND,) \
	_(ERROR_SYMBOL_NOT_FOUND,) \
	_(ERROR_RESOURCE_SHORTAGE,)
	
DEFINE_ENUM(ERROR_CODE, ENUM_LIST_ERROR_CODE)

const char* ERROR_CODE_AS_STRING(ERROR_CODE n);

#endif
