/*!
  \file		main.c
  \author	DilipSimha N M and Samuel
  \version 	3.0
  \date	
  			Created: Fri Sep 21, 2007  02:26PM
  			Last modified: Fri Oct 12, 2007  04:27PM
  \brief		
*/
#include <version.h>
#include <kernel/arch.h>
#include <kernel/debug.h>

/*! first C function which gets control from assembly
*/
void cmain()
{
	int a=0, b=1;
	ArchInit();
	kprintf( ACE_NAME" Version %d.%d Build%s\n", ACE_MAJOR, ACE_MINOR, ACE_BUILD);
	a = b/a;
}
