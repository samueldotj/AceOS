/*! \file	io.h
	\brief	read/write io ports
	\author	Samuel (samueldotj@gmail.com)
	\date	18-Jan-07 21:48
*/
#ifndef IO__H
#define IO__H

#include <ace.h>

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
