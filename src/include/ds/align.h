/*!
  \file	ds/align.h
  \author	Samuel (samueldotj@gmail.com)
  \version 	1.0
  \date	
  			Created: 22-Apr-2008 8:00PM
  			Last modified: 
  \brief	data structure alignment and padding macros
*/

#ifndef _ALIGH_H_
#define _ALIGH_H_

/*raises  given value to power of two and returns the value*/
#define POWER_OF_TWO( n )		( 1<< (n) )

/*retuns n-byte aligned value for the given address */
#define ALIGN_DOWN(addr, bytes)	((unsigned long)(addr) & -(POWER_OF_TWO( bytes )))

/*retuns n-byte up aligned value for the given address */
#define ALIGN_UP(addr, bytes)	((unsigned long)((addr) + POWER_OF_TWO(bytes)-1) & -POWER_OF_TWO(bytes))

#endif
