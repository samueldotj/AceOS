/*! Name: io.c
  Author: Samuel (samueldotj@gmail.com)
  Date: 18-01-07 21:42
  Description: Read or write value from/to hardware port.
*/

#include <ace.h>
#include <kernel/i386/io.h>

/*! Reads and returns a byte value from the specified port 
*/
inline BYTE _inp(WORD Port)
{
    BYTE Result;
    asm volatile("movw %1, %%dx;\
                 in %%dx, %%al;\
                 movb %%al, %0"
                :"=m"(Result)
                :"m"(Port)
                :"%eax","%edx");
    return Result;
}
/*! Reads and returns a word value from the specified port 
*/
inline WORD _inpw(WORD Port)
{
    WORD Result;
    asm volatile("movw %1, %%dx;\
                 in %%dx, %%ax;\
                 movw %%ax, %0"
                :"=m"(Result)
                :"m"(Port)
                :"%eax","%edx");
    return Result;

}
/*! Reads and returns a double word value from the specified port 
*/
inline DWORD _inpd(WORD Port)
{
    DWORD Result;
    asm volatile("movw %1, %%dx;\
                 in %%dx, %%eax;\
                 movl %%eax, %0"
                :"=m"(Result)
                :"m"(Port)
                :"%eax","%edx");
    return Result;

}

/*! Writes the given byte value to the specified port 
*/
inline void _outp(WORD Port, BYTE Value)
{
    asm volatile("movw %0, %%dx;\
                  movb %1, %%al;\
                  out %%al, %%dx;"
                : 
                :"m"(Port),"m"(Value)
                :"%eax","%edx");
}

/*! Writes the given word value to the specified port 
*/
inline void _outpw(WORD Port, WORD Value)
{
    asm volatile("movw %0, %%dx;\
                  movw %1, %%ax;\
                  out %%ax, %%dx;"
                : 
                :"m"(Port),"m"(Value)
                :"%eax","%edx");
}

/*! Writes the given double word value to the specified port 
*/
inline void _outpd(WORD Port, DWORD Value)
{
    asm volatile("movw %0, %%dx;\
                  movl %1, %%eax;\
                  out %%eax, %%dx;"
                : 
                :"m"(Port),"m"(Value)
                :"%eax","%edx");
}
