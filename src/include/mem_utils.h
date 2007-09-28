/*!
  \file		mem_utils.h
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created:
  			Last modified: Fri Sep 28, 2007  10:02AM
  \brief	
*/


#ifndef _MEM_UTILS_H_
#define _MEM_UTILS_H_

#ifdef __cplusplus
    extern "C" {
#endif

#include <mem_utils.h>

void* memchr(const void *s, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
void* memcpy(void *dest, const void *src, size_t n);
void* memmove(void *dest, void *src, size_t n);
void* memset(void *s, int c, size_t n);

#endif
