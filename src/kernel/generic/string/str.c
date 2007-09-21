/*! \file str.c
    \brief string utility functions
    
    Author : Samuel (samueldotj@gmail.com)
    Created Date : 12-Feb-2007 14:54
    
    This file should not depend on anyother functions such as malloc()
*/

#include <string.h>
#include <ctype.h>
#include <str.h>

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
char * str_get_token_info(char * src, unsigned int token_no, char token_separator, unsigned int * token_len)
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
char * str_get_token(char * buf, char * src, unsigned int token_no, char token_separator)
{
	int token_len;
	char * token = str_get_token_info( src, token_no, token_separator, &token_len);
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
char * str_ltrim(char * src)
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
char * str_rtrim(char * src)
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
char * str_atrim(char * src)
{
	return str_rtrim( str_ltrim( src ) );
}
/*!Converts the all alphabets in the string to upper case
*/
char * str_toupper(char * src)
{
	while( *src )
	{
		*src = toupper(*src);
		src++;
	}
}
/*!Converts the all alphabets in the string to lower case
*/
char * str_tolower(char * src)
{
	while( *src )
	{
		*src = tolower(*src);
		src++;
	}
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
