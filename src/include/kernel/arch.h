/*! \file arch.h
    \brief Architecture specific function declartions
	\author Samuel (samueldotj@gmail.com)
    \date 26/09/07 15:29
*/

#ifndef ARCH__H
#define ARCH__H

#include <ace.h>

#ifdef __cplusplus
    extern "C" {
#endif

void panic(char * message);
void ArchInit();
void ArchHalt();

#ifdef __cplusplus
	}
#endif


#endif
