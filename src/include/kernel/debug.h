/*!	\file	include/kernel/debug.h
	\brief	Debug function declartions

	Contains declarations of kernel trace, debug, assert functions/macros
*/

#ifndef DEBUG__H
#define DEBUG__H

#include <ace.h>
#include <stdarg.h>
#include <assert.h>
#include <kernel/printf.h>

/*define this macro to enable kernel tracing*/
#define __KERNEL_TRACE__

#ifdef __KERNEL_TRACE__
	#define KTRACE( ... )	\
			ktrace("%s:%d:%s(): ", __FILE__ , __LINE__,__PRETTY_FUNCTION__ ); \
			ktrace( __VA_ARGS__ );
#else
    #define KTRACE( ... ) 
#endif


#ifdef __cplusplus
	extern "C" {
#endif

extern void (*ktrace_putc)(void * arg, char ch);

int ktrace(const char *fmt, ...);

void panic(char * message);

/*architecture depended function declarations*/
void KtracePrint(void * arg, char ch);
void InitKtrace();

#ifdef __cplusplus
	}
#endif


#endif
