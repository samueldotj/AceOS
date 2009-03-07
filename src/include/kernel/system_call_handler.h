/*!
  \file     kernel/system_call_handler.h
  \brief    Contains system call handler
*/

#ifndef _SYSTEM_CALL_HANDLER_H_
#define _SYSTEM_CALL_HANDLER_H_

#include <ace.h>
#include <kernel/error.h>

#define SYSTEM_CALL_CACHE_FREE_SLABS_THRESHOLD	30
#define SYSTEM_CALL_CACHE_MIN_BUFFERS			50
#define SYSTEM_CALL_CACHE_MAX_SLABS				50

#define TOTAL_SYSTEM_CALL_ARGS 5

typedef struct system_call_args {
    UINT32 args[TOTAL_SYSTEM_CALL_ARGS];
}SYSTEM_CALL_ARGS, *SYSTEM_CALL_ARGS_PTR;

void SetupSystemCallHandler(void);

/* Below definitions in kernel/system_calls.c */
int SystemCallCacheConstructor(void *buffer);
int SystemCallCacheDestructor(void *buffer);
extern const int max_system_calls;
extern UINT32 ( *(system_calls[]) )(SYSTEM_CALL_ARGS_PTR, UINT32*);

#endif
