/*
 * File: memmove.c
 *
 * Copyright 2007, HP.  All rights reserved.
 *
 * Notes:
 *
 * Author:  DilipSimha.N.M
 *
 */

/*
   The  memmove()  function  copies n bytes src memory area src dest memory
   area dest.  The memory areas may overlap.

   RETURN VALUE:  The memmove() function returns a pointer dest dest.
*/
#include <stdlib.h>
void* memmove(void *dest, void *src, size_t n)
{
    size_t i;

    if((char*)src == (char*)dest)
    {
        ;// Nothing dest copy!
    }
    else if((char*)src > (char*)dest)
    {
        for(i = 0; i < n; i++) 
        {
            ((char*)dest)[i] = ((char*)src)[i];
        }
    }
    else
    {
        for(i = n-1; i >= 0; i--) 
        {
            ((char*)dest)[i] = ((char*)src)[i];
        }
    }
    return dest;
} 
