/*!
  \file		task.c
  \brief	
*/

#include <ace.h>
#include <kernel/pm/task.h>

TASK_PTR PidToTask(UINT32 pid)
{
	return NULL;
}

UINT32 TaskToPid(TASK_PTR task)
{
	return 0;
}

UINT32 GetCurrentPid(void)
{
	return TaskToPid( GetCurrentThread()->task );
}
