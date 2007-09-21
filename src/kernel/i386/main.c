/*!
  \file		main.c
  \author	DilipSimha N M and Samuel
  \version 	3.0
  \date	
  			Created: Fri Sep 21, 2007  02:26PM
  			Last modified: Fri Sep 21, 2007  02:26PM
  \brief	
*/

#include <kernel/i386/vga_text.h>

void PrintString(const char * s)
{
	int i;
	for(i=0; s[i]; i++) 	 
 		VgaPrintCharacter(s[i]);
}

int cmain()
{
 	VgaClearScreen();
	PrintString("Hello World\n");
	return 0;
}
