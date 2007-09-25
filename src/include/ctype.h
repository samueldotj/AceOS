/*! \file ctype.h
    \brief generic ctype
    
    This file contains common ascii check routines
    
    Author : Samuel (samueldotj@gmail.com)
*/
#ifndef CTYPE__H
#define CTYPE__H

#ifdef __cplusplus
	extern "C" {
#endif

#define isascii(c)  (c > 0 && c <= 0x7f)
#define iscntrl(c)  ((c > 0) && ((c < ' ') || (c == 0x7f)))
#define isdigit(c)  (c >= '0' && c <= '9')
#define isupper(c)  (c >=  'A' && c <= 'Z')
#define islower(c)  (c >=  'a' && c <= 'z')
#define isprint(c)  (c >= ' ' && c <= '~')
#define isspace(c)  (c == ' ' || c == '\f' || c == '\n' || c == '\r' ||\
                     c == '\t')
#define isalpha(c)  (isupper(c) || islower(c))
#define isalnum(c)  (isalpha(c) || isdigit(c))
#define isgraph(c)  (c != ' ' && isprint(c))
#define ispunct(c)  ((c > ' ' && c <= '~') && !isalnum(c))
#define isxupper(c) (isdigit(c) || (c >= 'A' && c <= 'F'))
#define isxlower(c) (isdigit(c) || (c >= 'a' && c <= 'f'))
#define isxdigit(c) (isxupper(c) || isxlower(c))
#define toascii(c)  (c & 0x7f)
#define toupper(c)  (islower(c) ? (c - ' ') : (c))
#define tolower(c)  (isupper(c) ? (c + ' ') : (c))

#define todigit(c) (c)<='0' ? 0   : ( (c>='9') ? 9:c-'0' )
#define tochar(n)  (n)>=9   ? '9' : ( (n<=0)   ? 0:n+'0' )

#ifdef __cplusplus
	}
#endif
#endif


