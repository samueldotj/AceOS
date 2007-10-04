/*!	\file	debug.h
	\brief	Debug function declartions
	\author	Samuel (samueldotj@gmail.com)
	\date	26/09/07 15:29

	Contains declarations of kernel trace, debug, assert functions/macros
*/

#ifndef DEBUG__H
#define DEBUG__H

#include <ace.h>
#include <stdarg.h>

/*define this macro to remove assert checking*/
//#define NDEBUG

#if defined(NDEBUG)
	#define assert(test) ((void)0) 
#else
	#define assert(test) ((void)((test)||(_assert(#test,__FILE__,__LINE__),0)))
#endif

/*define this macro to enable kernel tracing*/
#define __KERNEL_TRACE__

#ifdef __KERNEL_TRACE__
    #define KTRACE(msg) \
		ktrace("%s:%d:%s(): %s\n", __FILE__ , __LINE__,__PRETTY_FUNCTION__, msg );

    #define KTRACE1(format, para1) \
		ktrace("%s:%d:%s(): "format"\n", __FILE__ , __LINE__,__PRETTY_FUNCTION__, para1);

    #define KTRACE2(format, para1, para2) \
		ktrace("%s:%d:%s(): "format"\n", __FILE__ , __LINE__,__PRETTY_FUNCTION__, para1, para2);

    #define KTRACE3(format, para1, para2, para3) \
		ktrace("%s:%d:%s(): "format"\n", __FILE__ , __LINE__,__PRETTY_FUNCTION__, para1, para2, para3);
#else
    #define KTRACE(msg) 
    #define KTRACE1(format, para1) 
    #define KTRACE2(format, para1, para2) 
    #define KTRACE3(format, para1, para2, para3)
#endif


#ifdef __cplusplus
	extern "C" {
#endif

extern void (*kprintf_putc)(BYTE ch);
extern void (*ktrace_putc)(BYTE ch);

int _doprint(const char *fmt0, void (*putc)(BYTE ch), va_list argp);

int kprintf(const char *fmt, ...);
int ktrace(const char *fmt, ...);
void _assert(const char *msg, const char *file, int line);

/*architecture depended function declarations*/
void KtracePrint(BYTE ch);
void InitKtrace();

#ifdef __cplusplus
	}
#endif


#endif
