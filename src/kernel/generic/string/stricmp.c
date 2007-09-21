#include <string.h>
#include <ctype.h>

int stricmp(const char *s1, const char *s2)
{
  while (tolower(*s1) == tolower(*s2))
  {
    if (*s1 == 0)
    {
      return 0;
    }
    s1++;
    s2++;
  }
  return tolower(*s1) - tolower(*s2);
}
