/*!	\file string.h
	\brief standard string function declarations
	\author Samuel (samueldotj@gmail.com)
	\date 21/09/07 16:53
	This file contains all the common string function declaration. 
*/
#ifndef STRING_H
#define STRING_H

#include <stdlib.h>

#ifdef __cplusplus
    extern "C" {
#endif

void * memchr(const void *s, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
void* memcpy(void *dest, const void *src, size_t n);
void* memmove(void *dest, void *src, size_t n);
void* memset(void *s, int c, size_t n);

int str_total_characters(char * src, char ch);
int str_total_tokens(char * src, char ch);
char * str_get_token_info(char * src, unsigned int token_no, char token_separator, unsigned int * token_len);
char * str_get_token(char * buf, char * src, unsigned int token_no, char token_separator);
void str_replace(char * src, char oldch, char newch);
char* str_ltrim(char * src);
char* str_rtrim(char * src);
char* str_atrim(char * src);
char* str_toupper(char * src);
char* str_tolower(char * src);
int str_pattern_search(const char * src, const char * pattern);
char* strcat(char *s, const char *append);
char* strchr(const char *s, int c);
int strcmp(const char *s1, const char *s2);
int strcoll(const char *s1, const char *s2);
char* strcpy(char *dest, const char *src);
char* strncpy(char *dst, const char *src, size_t n);
size_t strspn(const char *s, const char *accept);
size_t strcspn(const char *s, const char *reject);
char* strerror(int errnum);
int stricmp(const char *s1, const char *s2);
size_t strlen(const char *str);
char* strncat(char *dest, const char *src, size_t n);
char *strnchr(const char *str, char c, size_t count);
int strncmp(const char *s1, const char *s2, size_t n);
char* strpbrk(const char *s1, const char *s2);
char* strstr(const char *s, const char *find);
char* strtok(char *s, const char *delim);
size_t strxfrm(char *dst, char *src, size_t n);

long strtol(const char *src, char **endptr, int base);
unsigned long strtoul(const char *src, char **endptr, int base);

#ifdef __cplusplus
	}
#endif

#endif
