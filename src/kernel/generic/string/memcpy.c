/*
 * File: memcpy.c
 *
 * Copyright 2007, HP.  All rights reserved.
 *
 * Notes:
 *
 * Author:  DilipSimha.N.M
 *
 */


/*The  memcpy()  function  copies  n bytes from memory area src to memory
  area dest.  The memory areas should not overlap.  Use memmove(3) if the
  memory areas do overlap.
  RETURN VALUE:     The memcpy() function returns a pointer to dest.
 */
#include <stdlib.h>

void* memcpy(void *dest, const void *src, size_t n)
{
    int i;
    for(i=0; i<n && ((char*)src)[i]; i++)
    {
        ((char*)dest)[i] = ((char*)src)[i];
    }
    return dest;
}
