#include <string.h>

#if 0
/*
   The strspn() function calculates the length of the initial segment of s which
   consists entirely of characters in accept.
   The strspn() function returns the number of characters in the initial segment
   of s which consist only of characters from accept.
*/

size_t strspn(const char *s, const char *accept)
{
    int i=0;
    char *c;

    for(i=0; s[i] != '\0'; i++;)
    {
        /*search in accept for any match with s[i].
          If matched then go to s[i+1].
          If not macthed, then go to next element in accept.
          If no elements in accept are matched then return 'i'.
          'i' will contain the count of longest match from start of s.
         */
        for(c = accept; (*c)!='\0'; c++)
        {
            if(*c == s[i]) {
                break;
            }
        }
        if(c == '\0') {
            break;
        }
    }
    return i;
}


/*
   The strcspn() function calculates the length of the initial segment of s which
   consists entirely of characters NOT in reject.
   The strcspn() function returns the number of characters in the initial
   segment of s which are not in the string reject.
*/

size_t strcspn(const char *s, const char *reject)
{
    int i=0;
    char *c;

    for(i=0; s[i] != '\0'; i++;)
    {
        /*search in accept for any match with s[i].
          If matched then go to s[i+1].
          If not macthed, then go to next element in accept.
          If no elements in accept are matched then return 'i'.
          'i' will contain the count of longest match from start of s.
         */
        for(c = accept; (*c)!='\0'; c++)
        {
            if(*c == s[i]) {
                break;
            }
        }
        if(c != '\0') {
            break;
        }
    }
    return i;
}

#endif
