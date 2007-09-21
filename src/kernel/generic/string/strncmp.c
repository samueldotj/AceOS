#include<stdlib.h>
/*
   The  function strcmp() compares the first (at most) n characters 
   of two strings s1 and s2.  
   It returns an integer less than, equal to, or greater than zero 
   if s1 is found, respectively, to be 
   less than, to match, or be greater than s2.
*/

int strncmp(const char *s1, const char *s2, size_t n)
{

    int ret = 0;
    while(n && (*s1) && (*s2))
    {
        if (*s1 != *s2)
        {
            break;
        }

        n--, s1++, s2++;
    }
    /*s1 or s2 cannot be null, only it contents can be NULL!*/
    return ((*(unsigned const char *)s1) - (*(unsigned const char *)s2));
}
