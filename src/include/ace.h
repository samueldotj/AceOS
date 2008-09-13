/*!	\file ace.h
	\brief Ace OS Specific Type Definitions
	This file contains all the common definitions which is different from standard C Lib and only applicable to Ace.
*/

#ifndef ACE__H
#define ACE__H

/*! Architecture name*/
#define ARCH	i386

/*! define this macro to enable SMP compilation*/
#define CONFIG_SMP

#define FALSE		0
#define TRUE  		1

typedef char CHAR;
typedef char INT8;
typedef unsigned char BYTE;
typedef unsigned char UINT8;
typedef BYTE BOOLEAN;

typedef short INT16;
typedef unsigned short UINT16;

typedef long INT32;
typedef unsigned long UINT32;

typedef long long INT64;
typedef unsigned long long UINT64;

typedef UINT32 VADDR;

#define BITS_PER_BYTE	(8)

#define BITS_PER_LONG ( 32 )

/*! Calculates a structures head address from given member address of the structure. It is useful if a linklist is in middle of a data structure.*/
#define STRUCT_ADDRESS_FROM_MEMBER(member_address, struct_name, member_name)	\
		((struct_name *)( (BYTE *)member_address - ((UINT32) &(((struct_name *)0)->member_name)) ))

/*! return the offset byte of a member from the structure starting.*/
#define OFFSET_OF_MEMBER(struct_name, member_name)	((UINT32) &(((struct_name *)0)->member_name))

/*! returns maximum of two numbers*/
#define MAX(a,b) (((a) > (b)) ? (a):(b))

typedef enum 
{
	LESS_THAN=-1,
	EQUAL=0,
	GREATER_THAN=1
}COMPARISION_RESULT;

#endif

