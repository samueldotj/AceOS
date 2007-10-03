/*!	\file ace.h
	\brief Ace OS Specific Type Definitions
	\author Samuel (samueldotj@gmail.com)
	\date 21/09/07 16:53
	This file contains all the common definitions which is different from standard C Lib and only applicable to Ace.
*/

#ifndef ACE__H
#define ACE__H

/*define this macro to enable SMP compilation*/
#define CONFIG_SMP


#define FALSE		0
#define TRUE  		1

typedef char CHAR;
typedef unsigned char BYTE;
typedef BYTE BOOLEAN;

typedef short INT16;
typedef unsigned short UINT16;

typedef long INT32;
typedef unsigned long UINT32;

typedef UINT32 WORD;
typedef UINT32 DWORD;

/*\def STRUCT_ADDRESS_FROM_MEMBER(member_address, struct_name, member_name)
	calculates a structures head address from given member address of the structure. It is useful if a linklist is in middle of a data structure.
*/
#define STRUCT_ADDRESS_FROM_MEMBER(member_address, struct_name, member_name)	\
		((struct_name *)( (BYTE *)member_address) - ((UINT32) &(((struct_name *)0)->member_name)) )

#endif

