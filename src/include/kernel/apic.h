/*!
  \file		apic.h
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created: Sat Jun 14, 2008  06:19PM
  			Last modified: Mon Jun 16, 2008  02:49PM
  \brief	
*/


#ifndef _APIC_H_
#define _APIC_H_

#include <ace.h>

int DetectApic();
void UpdateBaseApicAddress(UINT32 addr);
void UseApic(int enable);
IssueInterprocessorInterrupt();
INT32 GetApicId();
void InitAllApicRegisters(UINT32 base_address);

#endif
