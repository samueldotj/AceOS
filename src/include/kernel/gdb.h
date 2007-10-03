/*
GDB Stub Function declrations 
*/
#ifndef __GDB_H
#define __GDB_H

#include <ace.h>

void InitGdb();
inline int getDebugChar(void); /* read and return a single char */
inline void putDebugChar(int ch); /* write a single character      */
void exceptionHandler(int exc, void *addr); /* assign an exception handler   */
void flush_i_cache(void);

/* this function is used to set up exception handlers for tracing and  breakpoints */
void set_debug_traps (void);

extern UINT32 sys_gdb_port;

#endif
