/*!	\file	debug.h
	\brief	Debug function declartions

	Contains declarations of kernel trace, debug, assert functions/macros
*/

#ifndef DEBUG__H
#define DEBUG__H

#include <ace.h>
#include <stdarg.h>
#include <assert.h>

/*define this macro to enable kernel tracing*/
#define __KERNEL_TRACE__

#ifdef __KERNEL_TRACE__
	#define KTRACE( ... )	\
			ktrace("%s:%d:%s(): ", __FILE__ , __LINE__,__PRETTY_FUNCTION__ ); \
			ktrace( __VA_ARGS__ );
#else
    #define KTRACE(msg) 
#endif


#ifdef __cplusplus
	extern "C" {
#endif

extern void (*kprintf_putc)(BYTE ch);
extern void (*ktrace_putc)(BYTE ch);

int _doprint(const char *fmt0, void (*putc)(BYTE ch), va_list argp);

int kprintf(const char *fmt, ...);
int ktrace(const char *fmt, ...);

void panic(char * message);

/*architecture depended function declarations*/
void KtracePrint(BYTE ch);
void InitKtrace();

#ifdef __cplusplus
	}
#endif


#endif
