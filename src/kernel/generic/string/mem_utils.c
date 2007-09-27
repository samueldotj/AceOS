#include <stdlib.h>
#include <mem_utils.h>

void* memchr(const void *s, int c, size_t n)
{
        if (n)
        {
        const char *p = s;
        char cc = c;
        do {
                if (*p == cc)
                        return (void*) p;
                p++;
        } while (--n != 0);
        }
        return 0;
}


int memcmp(const void *s1, const void *s2, size_t n)
{
  if (n != 0)
  {
    const unsigned char *p1 = s1, *p2 = s2;

    do {
      if (*p1++ != *p2++)
        return (*--p1 - *--p2);
    } while (--n != 0);
  }
  return 0;
}


/*The  memcpy()  function  copies  n bytes from memory area src to memory
  area dest.  The memory areas should not overlap.  Use memmove(3) if the
  memory areas do overlap.
  RETURN VALUE:     The memcpy() function returns a pointer to dest.
 */
void* memcpy(void *dest, const void *src, size_t n)
{
    int i;
    for(i=0; i<n && ((char*)src)[i]; i++)
    {
        ((char*)dest)[i] = ((char*)src)[i];
    }
    return dest;
}


/*
   The  memmove()  function  copies n bytes src memory area src dest memory
   area dest.  The memory areas may overlap.

   RETURN VALUE:  The memmove() function returns a pointer dest dest.
*/
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


/*
   The  memset()  function  fills  the  first  n  bytes of the memory area
   pointed to by s with the constant byte c.

   RETURN VALUE:   The memset() function returns a pointer to the memory area s.
 */
void* memset(void *s, int c, size_t n)
{
    int i;
    for(i=0; i<n; i++) {
        ((char*)s)[i] = c;
    }
    return s;
}

