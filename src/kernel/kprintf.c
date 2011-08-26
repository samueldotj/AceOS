/*!
	\file		src/kernel/kprintf.c	
	\brief	kernel printf routines
*/
#include <ace.h>
#include <stdlib.h>
#include <kernel/debug.h>
#include <kernel/printf.h>
#include <kernel/mm/kmem.h>

typedef struct sprintf_argument SPRINTF_ARGUMENT, * SPRINTF_ARGUMENT_PTR;
struct sprintf_argument
{
	char * 	str;					/*starting address of the string buffer*/
	int 	current_position;		/*location to put incoming character*/
};

/*! function pointer used by kprintf() to write character*/
void (*kprintf_putc)(void * arg, char ch);

/*! puts the given character in the sprintf string in correct place*/
void sprintf_putc(void * arg, char ch)
{
	SPRINTF_ARGUMENT_PTR sprintf_arg = (SPRINTF_ARGUMENT_PTR)arg;
	sprintf_arg->str[sprintf_arg->current_position++] = ch;
	sprintf_arg->str[sprintf_arg->current_position] = 0;
}

/*! print given string and arguments in console
*/
int kprintf(const char *fmt, ...)
{
	#ifdef __KERNEL_TRACE__
		_doprint( fmt, ktrace_putc, NULL, (va_list) ((&fmt)+1));
	#endif
	return _doprint( fmt, kprintf_putc, NULL, (va_list) ((&fmt)+1));
}
/*! prints formatted string in a string buffer
*/
int sprintf(char * str, const char *fmt, ...)
{
	SPRINTF_ARGUMENT_PTR arg = kmalloc( sizeof(SPRINTF_ARGUMENT), 0 );
	if ( arg == NULL )
		return 0;
	arg->str = str;
	arg->current_position = 0;
	arg->str[arg->current_position] = 0;
	return _doprint( fmt, sprintf_putc, arg, (va_list) ((&fmt)+1));
}
