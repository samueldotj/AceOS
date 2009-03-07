/*!
  \file		kernel/pm/pid.c
  \brief	Process ID management
*/

#include <ace.h>
#include <string.h>
#include <ds/avl_tree.h>
#include <sync/spinlock.h>
#include <kernel/debug.h>
#include <kernel/mm/kmem.h>
#include <kernel/pm/pid.h>

SPIN_LOCK		pid_info_lock;	/*! lock to protect pid data structures*/
PID_INFO		pid_zero;		/*! zero is reserved for kernel, and it is always present*/
AVL_TREE_PTR	pid_root;		/*! root of pid avl tree*/

/*! internal function - Initializes the PidInfo data; called by slaballocator*/
int PidCacheConstructor(void *buffer)
{
	PID_INFO_PTR p = (PID_INFO_PTR) buffer;
	memset(p, 0, sizeof(PID_INFO) );
	InitAvlTreeNode( &pid_zero.tree_node, 0 );
	InitList( &p->inuse_list );
	InitList( &p->free_list );
	return 0;
}
/*! internal function - Reinitializes the PidInfo data; called by slaballocator
\todo - modify to just reinitialize only required field.*/
int PidCacheDestructor(void *buffer)
{
	return PidCacheConstructor( buffer );
}
/*! internal function - compares the given two pid info structures - called by avl tree*/
COMPARISION_RESULT compare_pid_info(struct binary_tree * node1, struct binary_tree * node2)
{
	int n1, n2;
	
	assert( node1 != NULL );
	assert( node2 != NULL );
	
	n1 = STRUCT_ADDRESS_FROM_MEMBER(STRUCT_ADDRESS_FROM_MEMBER(node1, AVL_TREE, bintree), PID_INFO, tree_node)->pid;
	n2 = STRUCT_ADDRESS_FROM_MEMBER(STRUCT_ADDRESS_FROM_MEMBER(node2, AVL_TREE, bintree), PID_INFO, tree_node)->pid;
	
	if ( n1 < n2 )
		return GREATER_THAN;
	else if ( n1 > n2 )
		return LESS_THAN;
	else 
		return EQUAL;
}
/*! Initializes the PID global variables*/
void InitPid()
{
	InitSpinLock( &pid_info_lock );
	pid_root = NULL;
	
	PidCacheConstructor( &pid_zero );
	pid_zero.free_count = MAX_PROCESS_ID-1;
}
/*! Allocate PID info structure
	\param ppid - parent process id
	\return on success - pid info structure
			on failure - null
*/
PID_INFO_PTR AllocatePidInfo(int ppid)
{
	PID_INFO_PTR pid_info, pid_free;

	pid_info = AllocateBuffer(&pid_cache, CACHE_ALLOC_SLEEP);
	if ( pid_info == NULL )
		return NULL;

	SpinLock( &pid_info_lock );
	
	pid_free = STRUCT_ADDRESS_FROM_MEMBER( pid_zero.free_list.next, PID_INFO, free_list );

	assert( pid_free != NULL );
	assert( pid_free->free_count > 0 );
	pid_free->free_count--;
	
	/*use the first available pid*/
	pid_info->pid = pid_free->pid + 1;
	pid_info->ppid = ppid;
	pid_info->free_count = pid_free->free_count;
	if ( pid_free->free_count > 0 )
		AddToList( &pid_free->free_list, &pid_info->free_list );
	RemoveFromList( &pid_free->free_list );

	/*add to the used pid list*/
	AddToList( &pid_free->inuse_list, &pid_info->inuse_list );
	/*add to the tree*/
	InsertNodeIntoAvlTree( &pid_root, &pid_info->tree_node, 0, compare_pid_info );
	SpinUnlock( &pid_info_lock );

	return pid_info;
}

void FreePidInfo(PID_INFO_PTR pid_info)
{
	PID_INFO_PTR prev_used_pid, prev_free_pid;

	assert( pid_info != NULL );
	assert( pid_info != &pid_zero );

	SpinLock( &pid_info_lock );
	prev_used_pid = STRUCT_ADDRESS_FROM_MEMBER( &pid_info->inuse_list.prev,	PID_INFO, inuse_list.prev );
	prev_free_pid = STRUCT_ADDRESS_FROM_MEMBER( &pid_info->free_list.prev,	PID_INFO, free_list.prev );

	/*transfer free pids to previous node*/
	prev_used_pid->free_count = pid_info->free_count + 1;
	if ( pid_info->free_count > 0 )
	{
		AddToList( &prev_used_pid->free_list, &pid_info->free_list );
		RemoveFromList(  &pid_info->free_list );
	}
	else
		AddToList( &prev_used_pid->free_list, &pid_zero.free_list );

	RemoveFromList(  &pid_info->inuse_list );
	RemoveNodeFromAvlTree( &pid_root, &pid_info->tree_node, 0, compare_pid_info);
	SpinUnlock( &pid_info_lock );

	FreeBuffer( pid_info, &pid_cache );
}
/*! Finds and returns Task from the given pid
	\param pid - process id
	\return On success - task
			On failure - null
*/
TASK_PTR PidToTask(int pid)
{
	PID_INFO search_pid;
	AVL_TREE_PTR result;
	
	search_pid.pid = 0;
	result = SearchAvlTree( pid_root, &search_pid.tree_node, compare_pid_info );
	if ( result == NULL )
		return NULL;
	return STRUCT_ADDRESS_FROM_MEMBER( result, PID_INFO, tree_node )->task;
}
