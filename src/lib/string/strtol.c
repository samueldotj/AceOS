/*!
  \file     strtol.c
  \brief	String to number conversion functions
*/

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

static int get_digit(char c, int base);
static char * find_base_and_sign(const char * src, int * sign, int * base);
static unsigned long calculate_value(char * src, int sign, int base);

long strtol(const char *src, char **endptr, int base)
{
	int sign = 1;
	long value=0;
	
	src = (char *)find_base_and_sign(src, &sign, &base);
	value = (long)calculate_value( (char *)src, sign, base );
	/*update the end pointer if given*/
	if ( endptr )
		*endptr = (char *)src;
	
	return (sign * value);
}
unsigned long strtoul(const char *src, char **endptr, int base)
{
	int sign = 1;
	unsigned long value=0;
	
	src = (char *)find_base_and_sign(src, &sign, &base);
	value = (unsigned long)calculate_value( (char *)src, sign, base );
	/*update the end pointer if given*/
	if ( endptr )
		*endptr = (char *)src;
	
	return (sign * value);
}
static unsigned long calculate_value(char * src, int sign, int base)
{
	unsigned long value = 0;
	/*calculate the absolute value*/
	while( isxdigit(src[0]) )
	{
		int digit =	get_digit(src[0], base);
		if ( digit < 0 )
			break;
		value = (value * base) + digit;
			
		src++;
	}
	return value;
}

static char * find_base_and_sign(const char * src, int * sign, int * base)
{
	/*skip the whitespaces*/
	while( isspace(src[0]) ) src++;
	
	/*find sign*/
	*sign = 1;
	if ( src[0] == '-' )
	{
		src++;
		*sign = -1;
	}
	
	/*find the base if it is not specified*/
	if ( *base == 0 )
	{
		if ( src[0]=='0' )
		{
			src++;
			if ( toupper(src[0])=='X' )
			{
				src++;
				*base = 16;
			}
			else if ( toupper(src[0])=='B' )
			{
				src++;
				*base = 2;
			}
			else
				*base = 8;
		}
		else
			*base = 10;
	}
	
	return (char *)src;
}

static int get_digit(char c, int base)
{
	int result;
	result	= c - '0';
	
	if ( result < 0 )
		return -1;
	else if ( result > 9 )
	{
		result = 10 + (toupper(c) - 'A');
	}
	
	if ( result >= base )
		return -1;
		
	return result;
}
