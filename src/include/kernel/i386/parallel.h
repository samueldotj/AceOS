/*! \file	include/kernel/i386/parallel.h
	\brief	parallel port read and write function declarations
*/
#ifndef PARALLEL__H
#define PARALLEL__H

#include <ace.h>

#ifdef __cplusplus
	extern "C" {
#endif

int InitParallelPort(int PortNo);
int WriteParallelPort(int PortNo, BYTE Character);

#ifdef __cplusplus
	}
#endif

#endif
