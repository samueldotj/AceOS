/*! \file	version.h
	\author	Samuel (samueldotj@gmail.com)
    \date	16-Jan-2007 10:12PM
	\brief	Ace OS version macros
	The ACE_BUILD (build number) will be automatically overwritten on each successful compilation
*/

#include <ace.h>

/*version of Ace*/
#define ACE_MAJOR 3
#define ACE_MINOR 0
#define ACE_BUILD " 282 (17-Apr-2008 11:30)"

#ifdef CONFIG_SMP
	#define ACE_NAME "Ace (Multiprocessor Compilation)"
#else
	#define ACE_NAME "Ace (Singleprocessor Compilation)"
#endif
