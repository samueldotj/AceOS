/*! \file debug.h
    \brief Debug function declartions
	\author Samuel (samueldotj@gmail.com)
    \date 26/09/07 15:29

    Contains declarations of kernel trace, debug, assert functions/macros
*/

#ifndef ARCH__H
#define ARCH__H

#include <ace.h>

#ifdef __cplusplus
    extern "C" {
#endif

void ArchInit();
void ArchHalt();

#ifdef __cplusplus
	}
#endif


#endif
