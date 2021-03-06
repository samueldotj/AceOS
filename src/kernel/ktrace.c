/*!
	\file		src/kernel/ktrace.c	
	\brief	ktrace and assert are defined here.
	Note - calls to these functions should made only after calling InitKtrace()
*/
#include <stdlib.h>
#include <kernel/debug.h>
#include <kernel/arch.h>
#include <kernel/printf.h>

#define MAX_STACK_FRAMES	50

/*! function pointer is used by the ktrace() to write characters 
*/
void (*ktrace_putc)(void * arg, char ch) = NULL;

/*! prints the given message in configured debug ports see ktrace.c in arch dir
*/
int ktrace(const char *fmt, ...)
{
	return _doprint( fmt, ktrace_putc, NULL, (va_list) ((&fmt)+1));
}

/*! assert function
Halts the system after printing the message. Used by assert macro
*/
void _assert(const char *msg, const char *file, int line)
{
	kprintf("Assertion failed at %s:%d::[%s]\n", file, line, msg);
	panic("Assert failed");
}

/*! Halts the system after printing the given message
*/
void panic(char * message) 
{
	static int panic=0;
	panic++;
	if( message ) 
	{
		kprintf("panic() : %s\n", message);
	}
	/*!avoid stack trace if we are already panicing*/
	if( panic <=1 )
	{
		PrintStackTrace(MAX_STACK_FRAMES);
	}
	ArchShutdown();
}
