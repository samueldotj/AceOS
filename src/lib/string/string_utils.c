/*!
  \file     string_utils.c
  \brief	This file contains all utility functions for string related operations inside kernel.
*/

#include <stdlib.h>
#include <ctype.h>
#include <string.h>


/*!Returns the number of occurence of a character in a string
*/
int str_total_characters(char * src, char ch)
{
	int i=0;
	while( *src )
	{
		if ( *src == ch )
			i++;
		
		src++;
	}
	return i;
}

/*!Returns total number of occurrence of a character in a string.

This function will omit the consecutive characters
for definition of token see str_get_token_info()
*/
int str_total_tokens(char * src, char ch)
{
    int i, result=0;
    char prev=0;
    for(i=0; src[i]; i++)
    {
        if ( src[i] == ch && prev != src[i])
            result++;
        prev = src[i];
    }
    /*if the string not ends with separator then we have one more token*/
    if ( prev != ch )
    	result++;

    return result;
}


/*!Returns information(position, length) about a token in a string.

A token is defined as group of characters separated by a special character.
The returned token_len is length of the token not including the terminating null.
Token numbering starts from 0
*/
char* str_get_token_info(char * src, unsigned int token_no, char token_separator, unsigned int * token_len)
{
	unsigned int i=0;

	/*find the start position of the token*/
	while( i < token_no ) 
	{
		char * start = strchr( src, token_separator );
		/*skip consecutive separators*/
		while( start && *start==token_separator )
			start++;
		if ( start == NULL )
			return NULL;
		
		src = start;

		i++;
	}

	/*find and update the token size*/
	for(i=0; src[i] && src[i] != token_separator; i++);
	*token_len = i;
		
	return src;
}

/*!Copies a specified token from the string into the buffer.

The buffer should have enough space to fill the token.
for definition of token see str_get_token_info()
*/
char* str_get_token(char * buf, char * src, unsigned int token_no, char token_separator)
{
	int unsigned token_len;
	char *token = str_get_token_info( src, token_no, token_separator, &token_len);
	if ( token )
	{
		strncpy( buf, token, token_len);
		buf[token_len] = 0;
		return buf;
	}
	else
	{
		buf[0] = 0;
		return NULL;
	}
}

/*!replace a character with the given character 
*/
void str_replace(char * src, char oldch, char newch)
{
	int i;
	//cant replace null character
	if ( oldch == 0 )
		return;
	for(i=0; src[i]; i++)
		if ( src[i] == oldch )
			src[i] = newch;
}

/*!Remove the space characters in the left from a string
*/
char* str_ltrim(char * src)
{
    int i,j;
    if ( src[0] == 0 )
        return NULL;
    
    for(i=0; src[i] == ' '; i++);
    if ( i!=0 )
    {
        for(j=0; src[i]; i++,j++)
            src[j] = src[i];
            
        src[j]=0;
    }
    return src;
}

/*!Remove the space characters in the right from a string
*/
char* str_rtrim(char * src)
{
    int i;
    if ( src[0] == 0 )
        return NULL;
    
    for(i=strlen(src)-1; src[i] == ' '; i--);
    src[i+1]=0;
    return src;
}

/*!Remove the space characters in the right and left from a string
*/
char* str_atrim(char * src)
{
	return str_rtrim( str_ltrim( src ) );
}


/*!Converts all the alphabets in the string to upper case
*/
char* str_toupper(char * src)
{
	char *ret = src;
	while( *src )
	{
		*src = toupper(*src);
		src++;
	}
	return ret;
}

/*!Converts the all alphabets in the string to lower case
*/
char* str_tolower(char * src)
{
	char *ret = src;
	while( *src )
	{
		*src = tolower(*src);
		src++;
	}
	return ret;
}

/*! searches for a pattern in the given string

Returns 1 on sucess else 0
*/
int str_pattern_search(const char * src, const char * pattern)
{
    int  i, slraw;
    
    /*  if it is end of both then strings match */
    if ((*pattern == '\0') && (*src == '\0'))
        return 1;  
    /*  if it is end of only pattern then mismatch*/
    if (*pattern == '\0')  
        return 0;
    
    /*  if pattern is a '*'    */
    if (*pattern == '*')
    {
		/*  if it is end of pattern then match */
        if (*(pattern+1) == '\0')
            return 1;
        /*    else hunt for match or wild card   */
        for(i=0,slraw=strlen(src);i<=slraw;i++)    
            if ((*(src+i) == *(pattern+1)) || (*(pattern+1) == '?'))
                if ( str_pattern_search(src+i+1, pattern+2) == 1)    /*  if found, match rest of pattern   */
                    return 1;
    }
    else
    {
		/*  if end of src then mismatch   */
        if ( *src == '\0' )
            return 0;
		/*  if chars match then try & match rest of it*/
        if ( (*pattern == '?') || (*pattern == *src) )
			if ( str_pattern_search(src+1,pattern+1) == 1)
               return 1;
    }
    
    /*no match found*/
    return 0;                                                  
}


char* strcat(char *s, const char *append)
{
  char *save = s;

  for (; *s; ++s);
  while ((*s++ = *append++));
  return save;
}


char* strchr(const char *s, int c)
{
  char cc = c;
  while (*s)
  {
    if (*s == cc)
      return (char*) s;
    s++;
  }
  if (cc == 0)
  {
    return (char*) s;
  }
  return 0;
}


int strcmp(const char *s1, const char *s2)
{
  while (*s1 == *s2)
  {
    if (*s1 == 0)
      return 0;
    s1++;
    s2++;
  }
  return *(unsigned const char *)s1 - *(unsigned const char *)(s2);
}

int strcoll(const char *s1, const char *s2)
{
  return strcmp(s1, s2);
}


char* strcpy(char *dest, const char *src)
{
    int i;
    for(i=0; src[i] != '\0'; i++)
    {
        dest[i] = src[i];
    }
    dest[i] = '\0';
    return dest;
}

char* strncpy(char *dst, const char *src, size_t n)
{
  if (n != 0) {
    char *d = dst;
    const char *s = src;

    do {
      if ((*d++ = *s++) == 0)
      {
        while (--n != 0)
          *d++ = 0;
        break;
      }
    } while (--n != 0);
  }
  return dst;
}


/*
   The strspn() function calculates the length of the initial segment of s which
   consists entirely of characters in accept.
   The strspn() function returns the number of characters in the initial segment
   of s which consist only of characters from accept.
*/

size_t strspn(const char *s, const char *accept)
{
    int i=0;
    const char *c;

    for(i=0; s[i] != '\0'; i++)
    {
        /*search in accept for any match with s[i].
          If matched then go to s[i+1].
          If not macthed, then go to next element in accept.
          If no elements in accept are matched then return 'i'.
          'i' will contain the count of longest match from start of s.
         */
        for(c = accept; (*c)!='\0'; c++)
        {
            if(*c == s[i]) {
                break;
            }
        }
        if(c == '\0') {
            break;
        }
    }
    return i;
}


/*
   The strcspn() function calculates the length of the initial segment of s which
   consists entirely of characters NOT in reject.
   The strcspn() function returns the number of characters in the initial
   segment of s which are not in the string reject.
*/

size_t strcspn(const char *s, const char *reject)
{
    int i=0;
    const char *c;

    for(i=0; s[i] != '\0'; i++)
    {
        /*search in accept for any match with s[i].
          If matched then go to s[i+1].
          If not macthed, then go to next element in accept.
          If no elements in accept are matched then return 'i'.
          'i' will contain the count of longest match from start of s.
         */
        for(c = reject; (*c)!='\0'; c++)
        {
            if(*c == s[i]) {
                break;
            }
        }
        if(c != '\0') {
            break;
        }
    }
    return i;
}



char* strerror(int errnum)
{
        return "string error : not supported";
}

int stricmp(const char *s1, const char *s2)
{
  while (tolower(*s1) == tolower(*s2))
  {
    if (*s1 == 0)
    {
      return 0;
    }
    s1++;
    s2++;
  }
  return tolower(*s1) - tolower(*s2);
}


size_t strlen(const char *str)
{
  const char *s;

  for (s = str; *s; ++s)
  {
      ;
  }

  return (s-str);
}



/*
   The strncat() function appends 'n' characters from src string to the
   dest string overwriting the '\0' character at the end of dest, and then adds a
   terminating '\0' character. Since the result is always terminated with '\0',
   at most n+1 characters are written.

   RETURN VALUE
   The strncat() functions return a pointer to the resulting string dest.

*/

char* strncat(char *dest, const char *src, size_t n)
{
  char *dest_temp = dest;

  if (n != 0)
  {
    while (*dest != 0)
    {
      dest++;
    }

    while(n!=0 && (*src))
    {
      *dest = *src;
      dest++, src++, n--;
    }
    *dest = '\0';
  }

  return dest_temp;
}



/*
   The function strnchr reports the first occurance of character 'c' inside
   string 'str' within count characters.
   Return value: If the character 'c' is found within count number of characters
   from the start of str, the pointer to that character is returned else NULL is
   returned.
*/

char *strnchr(const char *str, char c, size_t count)
{
  char *ptr = NULL;

  while(*str && count > 0)
  {
    if(*str == c)
    {
      ptr = (char*)str;
      break;
    }
    str++;
    count--;
  }
  return ptr;
}



/*
   The  function strncmp() compares the first (at most) n characters
   of two strings s1 and s2.
   It returns an integer less than, equal to, or greater than zero
   if s1 is found, respectively, to be
   less than, to match, or be greater than s2.
*/

int strncmp(const char *s1, const char *s2, size_t n)
{
	while(n && (*s1) && (*s2))
    {
		if (*s1 != *s2)
        {
            break;
        }

        n--;
		if ( n )
			s1++, s2++;
    }
    /*s1 or s2 cannot be null, only it contents can be NULL!*/
    return ((*(unsigned const char *)s1) - (*(unsigned const char *)s2));
}

char* strpbrk(const char *s1, const char *s2)
{
  const char *scanp;
  int c, sc;

  while ((c = *s1++) != 0)
  {
    for (scanp = s2; (sc = *scanp++) != 0;)
      if (sc == c)
    return (char*) s1 - 1;
  }
  return 0;
}


char* strstr(const char *s, const char *find)
{
  char c, sc;
  size_t len;

  if ((c = *find++) != 0)
  {
    len = strlen(find);
    do {
      do {
        if ((sc = *s++) == 0)
          return 0;
      } while (sc != c);
    } while (strncmp(s, find, len) != 0);
    s--;
  }
  return (char*) s;
}



char* strtok(char *s, const char *delim)
{
  const char *spanp;
  int c, sc;
  char *tok;
  static char *last;


  if (s == NULL && (s = last) == NULL)
    return (NULL);

  /*
   * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
   */
 cont:
  c = *s++;
  for (spanp = delim; (sc = *spanp++) != 0;) {
    if (c == sc)
      goto cont;
  }

  if (c == 0) {			/* no non-delimiter characters */
    last = NULL;
    return (NULL);
  }
  tok = s - 1;

  /*
   * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
   * Note that delim must have one NUL; we stop if we see that, too.
   */
  for (;;) {
    c = *s++;
    spanp = delim;
    do {
      if ((sc = *spanp++) == c) {
	if (c == 0)
	  s = NULL;
	else
	  s[-1] = 0;
	last = s;
	return (tok);
      }
    } while (sc != 0);
  }
  /* NOTREACHED */
}



size_t strxfrm(char *dst, char *src, size_t n)
{
  size_t r = 0;
  int c;

  if (n != 0) {
    while ((c = *src++) != 0)
    {
      r++;
      if (--n == 0)
      {
        while (*src++ != 0)
          r++;
        break;
      }
      *dst++ = c;
    }
    *dst = 0;
  }
  return r;
}
