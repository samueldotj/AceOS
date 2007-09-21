#include <string.h>
#include "syserr1.h"

char* strerror(int errnum)
{
  static char ebuf[40];		/* 64-bit number + slop */
  char *cp;
  int v=1000000, lz=0;

  if (errnum >= 0 && errnum < __sys_nerr)
    return (char*) __sys_errlist[errnum];

  strcpy(ebuf, "Unknown error: ");
  cp = ebuf + 15;
  if (errnum < 0)
  {
    *cp++ = '-';
    errnum = -errnum;
  }
  while (v)
  {
    int d = errnum / v;
    if (d || lz || (v == 1))
    {
      *cp++ = d+'0';
      lz = 1;
    }
    errnum %= v;
    v /= 10;
  }

  return ebuf;
}
