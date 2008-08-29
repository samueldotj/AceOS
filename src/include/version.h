/*! \file	version.h
	\brief	Ace OS version macros
*/

#include <ace.h>
#include <build.h>
/*! Major version of Ace*/
#define ACE_MAJOR 3
/*! Minor version of Ace*/
#define ACE_MINOR 0

#ifdef CONFIG_SMP
	#define ACE_NAME "Ace(MP) "
#else
	#define ACE_NAME "Ace"
#endif
