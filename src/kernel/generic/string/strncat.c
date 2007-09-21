#include <string.h>
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
  if (n != 0)
  {
    char *dest_temp = dest;

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
  return dest;
}
