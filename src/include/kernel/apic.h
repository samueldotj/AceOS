/*!
  \file		kernel/apic.h
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created: Sat Jun 14, 2008  06:19PM
  			Last modified: Thu Jul 24, 2008  03:38PM
  \brief	
*/


#ifndef _APIC_H_
#define _APIC_H_

#include <ace.h>

int DetectApic();
void UseApic(int enable);
INT32 GetApicId();
void RelocateBaseApicAddress(UINT32 addr);
#endif
