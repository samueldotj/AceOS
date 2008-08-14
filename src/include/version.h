/*! \file	version.h
	\brief	Ace OS version macros
*/

#include <ace.h>

/*! Major version of Ace*/
#define ACE_MAJOR 3
/*! Minor version of Ace*/
#define ACE_MINOR 0
/*! Build number - It will be automatically overwritten on each successful compilation*/
#define ACE_BUILD " 2 (14-Aug-2008 08:58)"

#ifdef CONFIG_SMP
	#define ACE_NAME "Ace (Multiprocessor Compilation)"
#else
	#define ACE_NAME "Ace (Singleprocessor Compilation)"
#endif
