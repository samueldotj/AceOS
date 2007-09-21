#include <stdlib.h>
/*
   The function strnchr reports the first occurance of character 'c' inside
   string 'str' within count characters.
   Return value: If the character 'c' is found within count number of characters
   from the start of str, the pointer to that character is returned else NULL is
   returned.
*/

char *strnchr(const char *str, char c, size_t count)
{
  char *ptr = NULL;

  while(*str && count > 0)
  {
    if(*str == c)
    {
      ptr = str;
      break;
    }
    str++;
    count--;
  }
  return ptr;
}
