/*!
\file	kernel/wait_event.h>
\brief	Wait event strcutures and handlers are declared here.
*/

#ifndef WAIT_EVENT_H
#define WAIT_EVENT_H

#include <ace.h>
#include <ds/list.h>


typedef struct wait_event WAIT_EVENT, *WAIT_EVENT_PTR;
#include <kernel/pm/task.h>

#define WAIT_EVENT_WAKE_UP_ALL 1

/*This is to make a list of related events */
#define ADD_MULTIPLE_WAIT_EVENTS(event_head, new_event) AddToListTail( &(event_head->thread_events), &(new_event->thread_events) );

struct wait_event
{
	THREAD_PTR		thread;			/*Pointer to thread which is waiting on this event */
	BYTE			fired;			/* Indicates if this event got fired(1) or just got removedi(0) because soe other related event fired */
	LIST			in_queue;		/*List of events in this queue bucket. Each of these events will point to different threads. */
	LIST			thread_events;	/* List of events which belong to same thread */
};

WAIT_EVENT_PTR AddToEventQueue(WAIT_EVENT_PTR *wait_queue);
UINT32 WaitForEvent(WAIT_EVENT_PTR event, UINT32 timeout);
void WakeUpEvent(WAIT_EVENT_PTR *event, int flag);
void RemoveEventFromQueue(WAIT_EVENT_PTR search_wait_event, WAIT_EVENT_PTR *wait_queue);

#endif
