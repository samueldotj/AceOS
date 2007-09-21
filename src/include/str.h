/*! \file str.h
    \brief utility string functions
    
    \author	Samuel (samueldotj@gmail.com)
    \date 21/09/07 18:27
*/
#ifndef STR_H
#define STR_H

#ifdef __cplusplus
    extern "C" {
#endif

int str_total_characters(char * src, char ch);
int str_total_tokens(char * src, char ch);

char * str_get_token_info(char * src, unsigned int token_no, char token_separator, unsigned int * token_len);
char * str_get_token(char * buf, char * src, unsigned int token_no, char token_separator);

void str_replace(char * src, char oldch, char newch);
char * str_ltrim(char * src);
char * str_rtrim(char * src);
char * str_atrim(char * src);
char * str_toupper(char * src);
char * str_tolower(char * src);

int str_pattern_search(const char * src, const char * pattern);

#ifdef __cplusplus
	}
#endif

#endif
