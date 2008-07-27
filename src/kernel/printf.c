/*!
	\file		printf.c
	\author		Samuel
	\version	3.0
	\date	
  			Created: 26/09/07 11:30
  			Last modified: 26/09/07 11:30
	\brief	printf implementation
	Copied and modified the doprnt.c file provided with DJGPP
	This prints doesnt support floating point, 64bit division and mod operations
*/

#include <ctype.h>
#include <string.h>
#include <stdarg.h>

char decimal = '.';
int nan = 0;
/* 11-bit exponent (VAX G floating point) is 308 decimal digits */
#define	MAXEXP		308
#define MAXEXPLD	100 /* this includes subnormal numbers */
//#define	MAXEXP		308
//#define MAXEXPLD	4952 /* this includes subnormal numbers */
/* 128 bit fraction takes up 39 decimal digits; max reasonable precision */
#define	MAXFRACT	39

#define	DEFPREC		6
#define	DEFLPREC	6

#define	BUF			(MAXEXPLD+MAXFRACT+1)	/* + decimal point */

#define ARG(basetype) _ulonglong = \
		flags&LONGDBL ? va_arg(argp, long long basetype) : \
		flags&LONGINT ? va_arg(argp, long basetype) : \
		flags&SHORTINT ? (short basetype)va_arg(argp, int) : \
		va_arg(argp, int)

/* have to deal with the negative buffer count kludge */

#define	LONGINT		0x01		/* long integer */
#define	LONGDBL		0x02		/* long double */
#define	SHORTINT	0x04		/* short integer */
#define	ALT			0x08		/* alternate form */
#define	LADJUST		0x10		/* left adjustment */
#define	ZEROPAD		0x20		/* zero (as opposed to blank) pad */
#define	HEXPREFIX	0x40		/* add 0x or 0X prefix */


int _doprint(const char *fmt0, void (*putc)(char ch), va_list argp);

/*kernel doesnt support 64 bit division*/
int __udivdi3()
{
	return 0;
}
/*kernel doesnt support 64 bit mod*/
int __umoddi3()
{
	return 0;
}


static char NULL_REP[] = "(null)";

int _doprint(const char *fmt0, void (*putc)(char ch), va_list argp)
{	
	const char *fmt;		/* format string */
	int ch,mch;					/* character from fmt */
	int cnt;				/* return value accumulator */
	int n;					/* random handy integer */
	char *t;				/* buffer pointer */
	unsigned long long _ulonglong=0; /* integer arguments %[diouxX] */
	int base;				/* base for [diouxX] conversion */
	int dprec;				/* decimal precision in [diouxX] */
	int fieldsz;			/* field size expanded by sign, etc */
	int flags;				/* flags as above */
	int fpprec;				/* `extra' floating precision in [eEfgG] */
	int prec;				/* precision from format (%.3d), or -1 */
	int realsz;				/* field size expanded by decimal precision */
	int size;				/* size of converted field or string */
	int width;				/* width from format (%8d), or 0 */
	char sign;				/* sign prefix (' ', '+', '-', or \0) */
	const char *digs;		/* digits for [diouxX] conversion */
	char buf[BUF];			/* space for %c, %[diouxX], %[eEfgG] */
	decimal = '.';
	fmt = fmt0;
  	digs = "0123456789abcdef";
  	for (cnt = 0;; ++fmt)
	{
    	while ((ch = *fmt) && ch != '%')
   		{
			putc (ch);
	      	fmt++;
      		cnt++;
      	}
	   	if (!ch)
      		return cnt;
   		flags = 0; dprec = 0; fpprec = 0; width = 0;
   		prec = -1; sign = '\0';
rflag:
		mch = *++fmt;
 		if ( mch == ' ' )
 		{	
			/*
			* ``If the space and + flags both appear, the space
			* flag will be ignored.''
			*	-- ANSI X3J11
			*/
			if (!sign)
				sign = ' ';
       		goto rflag;
		}
		else if ( mch == '#' )
		{
			flags |= ALT;
			goto rflag;
		}
		else if ( mch == '*' || mch == '-' )
		/*
			 * ``A negative field width argument is taken as a
		* - flag followed by a  positive field width.''
		*	-- ANSI X3J11
		* They don't exclude field widths read from args.
		*/
		{
			if ((width = va_arg(argp, int)) >= 0)
				goto rflag;
			width = -width;
			if ( mch == '-' )
			{
				/* FALLTHROUGH */
				flags |= LADJUST;
				goto rflag;
			}
		}
		else if ( mch == '+' )
		{
			sign = '+';
			goto rflag;
		}
		else if ( mch == '.' )
		{
			if (*++fmt == '*')
				n = va_arg(argp, int);
			else
			{
				n = 0;
				do 
				{
					n = 10 * n + todigit(*fmt);
					++fmt;
				} while (isascii(*fmt) && isdigit(*fmt));
				--fmt;
			}
			prec = n < 0 ? -1 : n;
			goto rflag;
		}
		else if ( mch == '0' )
		{
			/*
			* ``Note that 0 is taken as a flag, not as the
			* beginning of a field width.''
			*	-- ANSI X3J11
			*/
			flags |= ZEROPAD;
			goto rflag;
		}
		else if ( mch >= '1' && mch <= '9' )
		{
			n = 0;
			do 
			{
				n = 10 * n + todigit(*fmt);
				++fmt;
			} while (isascii(*fmt) && isdigit(*fmt));
			width = n;
			--fmt;
			goto rflag;
		}
		else if ( mch == 'L' )
		{
			flags |= LONGDBL;
			goto rflag;
		}
		else if ( mch == 'h' )
		{
			flags |= SHORTINT;
			goto rflag;
		}
		else if ( mch == 'l' )
		{
			if (flags&LONGINT)
				flags |= LONGDBL; /* for 'll' - long long */
			else
				flags |= LONGINT;
			goto rflag;
		}
		else if ( mch == 'c' )
		{
			*(t = buf) = va_arg(argp, int);
			size = 1;
			sign = '\0';
			goto pforw;
		}
		else if ( mch == 'D' || mch =='d' || mch=='i' )
		{
			flags |= LONGINT;
			if ( mch =='d' || mch=='i' )
			{/*FALLTHROUGH*/
				ARG(int);
				if ((long long)_ulonglong < 0)
				{
					_ulonglong = -_ulonglong;
					sign = '-';
				}
				base = 10;
				goto number;
			}
		}
		else if ( mch=='n' )
		{
			if (flags & LONGDBL)
				*va_arg(argp, long long *) = cnt;
			else if (flags & LONGINT)
				*va_arg(argp, long *) = cnt;
			else if (flags & SHORTINT)
				*va_arg(argp, short *) = cnt;
			else
				*va_arg(argp, int *) = cnt;
		}
		else if ( mch=='O' || mch=='o' )
		{
			if (mch=='o')
				flags |= LONGINT;
			ARG(unsigned);
			base = 8;
			goto nosign;
		}
		else if ( mch=='p' )
		{			
			/*
			* ``The argument shall be a pointer to void.  The
			* value of the pointer is converted to a sequence
			* of printable characters, in an implementation-
			* defined manner.''
			*	-- ANSI X3J11
			*/
			/* NOSTRICT */
			_ulonglong = (unsigned long)va_arg(argp, void *);
			base = 16;
			goto nosign;
		}
		else if ( mch == 's' )
		{
			if (!(t = va_arg(argp, char *)))
				t = NULL_REP;
			if (prec >= 0)
			{
				/*
				* can't use strlen; can only look for the
				* NUL in the first `prec' characters, and
				* strlen() will go further.
				*/
				char *p			/*, *memchr() */;
				if ((p = strnchr(t, 0, prec)))
				{
					size = p - t;
					if (size > prec)
						size = prec;
				}
				else
					size = prec;
			}
			else
				size = strlen(t);
			sign = '\0';
			goto pforw;
		}
		else if ( mch == 'U' ||  mch=='u')
		{
			if ( mch == 'U' )
        		flags |= LONGINT;
			ARG(unsigned);
			base = 10;
			goto nosign;
		}
		else if ( mch=='X' || mch=='x' )
		{
			if (mch=='X' )
				digs = "0123456789ABCDEF";
			ARG(unsigned);
			base = 16;
			/* leading 0x/X only if non-zero */
			if (flags & ALT && _ulonglong != 0)
				flags |= HEXPREFIX;
nosign:
			/* unsigned conversions */
			sign = '\0';
number:
			/*
			* ``... diouXx conversions ... if a precision is
			* specified, the 0 flag will be ignored.''
			*	-- ANSI X3J11
			*/
			if ((dprec = prec) >= 0)
				flags &= ~ZEROPAD;
			/* ``The result of converting a zero value with an
			* explicit precision of zero is no characters.''
			*	-- ANSI X3J11*/
			t = buf + BUF;
			if (_ulonglong != 0 || prec != 0)
			{
				/* conversion is done separately since operations
				with long long are much slower */
				#define CONVERT(type) \
				{ \
				register type _n = (type)_ulonglong; \
				do { \
					*--t = digs[_n % base]; \
					_n /= base; \
					} while (_n); \
				}
				if (flags&LONGDBL)
					CONVERT(unsigned long long) /* no ; */
				else
					CONVERT(unsigned long) /* no ; */
				#undef CONVERT
				if (flags & ALT && base == 8 && *t != '0')
					*--t = '0';		/* octal leading 0 */
		    }

			digs = "0123456789abcdef";
			size = buf + BUF - t;
pforw:
			/*
			* All reasonable formats wind up here.  At this point,
			* `t' points to a string which (if not flags&LADJUST)
			* should be padded out to `width' places.  If
			* flags&ZEROPAD, it should first be prefixed by any
			* sign or other prefix; otherwise, it should be blank
			* padded before the prefix is emitted.  After any
			* left-hand padding and prefixing, emit zeroes
			* required by a decimal [diouxX] precision, then print
			* the string proper, then emit zeroes required by any
			* leftover floating precision; finally, if LADJUST,
			* pad with blanks.
			*/
		
			/*
			* compute actual size, so we know how much to pad
			* fieldsz excludes decimal prec; realsz includes it
			*/
			fieldsz = size + fpprec;
			realsz = dprec > fieldsz ? dprec : fieldsz;
			if (sign)
				realsz++;
			if (flags & HEXPREFIX)
				realsz += 2;
			/* right-adjusting blank padding */
			if ((flags & (LADJUST|ZEROPAD)) == 0 && width)
				for (n = realsz; n < width; n++)
					putc(' ');
			/* prefix */
			if (sign)
				putc(sign);
			if (flags & HEXPREFIX)
			{
				putc('0');
				putc((char)*fmt);
			}
			/* right-adjusting zero padding */
			if ((flags & (LADJUST|ZEROPAD)) == ZEROPAD)
				for (n = realsz; n < width; n++)
					putc('0');
			/* leading zeroes from decimal precision */
			for (n = fieldsz; n < dprec; n++)
				putc('0');

			/* the string or number proper */
			for (n = size; n > 0; n--)
				putc(*t++);
			/* trailing f.p. zeroes */
			while (--fpprec >= 0)
				putc('0');
			/* left-adjusting padding (always blank) */
			if (flags & LADJUST)
				for (n = realsz; n < width; n++)
					putc(' ');
			/* finally, adjust cnt */
			cnt += width > realsz ? width : realsz;
        }
		else if ( mch=='\0')			/* "%?" prints ?, unless ? is NULL */
			return cnt;
		else
		{
			putc((char)*fmt);
			cnt++;
		}
			
	}
  	/* NOT REACHED */
}
