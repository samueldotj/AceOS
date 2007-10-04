/*! \file io.h
    \brief Ace Kernel IO utility
    
    Author : Samuel (samueldotj@gmail.com)
    Created Date : 18-Jan-07 21:48
*/
#include <ace.h>

#ifndef __IO__H
#define __IO__H

#ifdef __cplusplus
    extern "C" {
#endif

BYTE _inp(UINT16 Port);
UINT16 _inpw(UINT16 Port);
UINT32 _inpd(UINT16 Port);

void _outp(UINT16 Port, BYTE Value);
void _outpw(UINT16 Port, UINT16 Value);
void _outpd(UINT16 Port, UINT32 Value);

#endif
