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

BYTE _inp(WORD Port);
WORD _inpw(WORD Port);
DWORD _inpd(WORD Port);

void _outp(WORD Port, BYTE Value);
void _outpw(WORD Port, WORD Value);
void _outpd(WORD Port, DWORD Value);

#endif
