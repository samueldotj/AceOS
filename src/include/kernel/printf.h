/*!	\file	printf.h
	\brief	Printf declarations
*/

#ifndef PRINTF__H
#define PRINTF__H

#include <ace.h>
#include <stdarg.h>
#include <assert.h>

extern void (*kprintf_putc)(void * arg, char ch);
int _doprint(const char *fmt0, void (*putc)(void * arg, char ch), void * putc_arg, va_list argp);

int kprintf(const char *fmt, ...);

int sprintf(char * str, const char *fmt, ...);

#endif
