/*!
  \file		main.c
  \author	DilipSimha N M and Samuel
  \version 	3.0
  \date	
  			Created: Fri Sep 21, 2007  02:26PM
  			Last modified: Fri Sep 21, 2007  02:26PM
  \brief	
*/
#include <version.h>

/*! first C function which gets control from assembly
*/
void cmain()
{
	ArchInit();
	kprintf( ACE_NAME" Version %d.%d Build%s\n", ACE_MAJOR, ACE_MINOR, ACE_BUILD);
}
