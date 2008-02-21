/*!	\file	debug.h
	\brief	Debug function declartions
	\author	Samuel (samueldotj@gmail.com)
	\date	26/09/07 15:29

	Contains declarations of kernel trace, debug, assert functions/macros
*/

#ifndef ASSERT__H
#define ASSERT__H
/*define this macro to remove assert checking*/
//#define NDEBUG

#if defined(NDEBUG)
	#define assert(test) ((void)0) 
#else
	#define assert(test) ((void)((test)||(_assert(#test,__FILE__,__LINE__),0)))
#endif

#ifdef __cplusplus
	extern "C" {
#endif

void _assert(const char *msg, const char *file, int line);

#ifdef __cplusplus
	}
#endif

#endif

