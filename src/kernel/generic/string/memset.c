/*
 * File: memset.c
 *
 * Copyright 2007, HP.  All rights reserved.
 *
 * Notes:
 *
 * Author:  DilipSimha.N.M
 *
 */

/*
   The  memset()  function  fills  the  first  n  bytes of the memory area
   pointed to by s with the constant byte c.

   RETURN VALUE:   The memset() function returns a pointer to the memory area s.
 */
#include <stdlib.h>
void* memset(void *s, int c, size_t n)
{
    int i;
    for(i=0; i<n; i++) {
        ((char*)s)[i] = c;
    }
    return s;
}
