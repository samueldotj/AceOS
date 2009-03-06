/*! \file include/kernel/pm/timeout_queue.h
    \brief Timeout queue related structrues and function declarations
*/

#ifndef _TIMEOUT_QUEUE_H_
#define _TIMEOUT_QUEUE_H_

#include <ace.h>
#include <ds/list.h>

typedef struct timeout_queue
{
    LIST        queue;
    INT32       sleep_time;
}TIMEOUT_QUEUE, *TIMEOUT_QUEUE_PTR;

extern volatile TIMEOUT_QUEUE_PTR timeout_queue;

INT32 Sleep(UINT32 timeout);
void ValuateTimeoutQueue(void);
int RemoveFromTimeoutQueue(void);

#endif
