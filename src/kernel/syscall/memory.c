/*!    \file   memory.c    \brief  system call related to virtual memory management*/#include <ace.h>#include <string.h>#include <kernel/debug.h>#include <kernel/system_call_handler.h>#include <kernel/mm/vm.h>/*! \todo - remove*/UINT32 syscall_brk(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	return -1;}/*! System call to allocate virtual memory to current user process	\input args[0] - size	\input args[1] - protection	\input args[2] - preferred va start*/UINT32 syscall_allocate_virtual_memory(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	UINT32 size, protection, preferred_va;	VADDR va;	ERROR_CODE err;			size = PAGE_ALIGN_UP(sys_call_args->args[0]);	protection = sys_call_args->args[1];	preferred_va = sys_call_args->args[2];		if ( size <= 0 )	{		*retval = NULL;		return ERROR_SUCCESS;	}		err = AllocateVirtualMemory( GetCurrentVirtualMap(), &va, preferred_va, size, protection, NULL, NULL ) ;	KTRACE("(size=%d protection=%d preferred_va=%p %d %d) VA=%p\n", size, protection, preferred_va,sys_call_args->args[3], sys_call_args->args[4], va );	if ( err == ERROR_SUCCESS )		*retval = va;	else		*retval = NULL;	return err;}UINT32 syscall_get_mempolicy(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	
	* retval = -1;
	return 0;}UINT32 syscall_set_mempolicy(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	KTRACE("%p %p %p %p", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;}UINT32 syscall_madvise(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;}UINT32 syscall_munmap(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;}UINT32 syscall_mremap(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;}UINT32 syscall_mmap2(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;}UINT32 syscall_mprotect(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;}UINT32 syscall_flock(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;}UINT32 syscall_msync(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;}UINT32 syscall_sysctl(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;}UINT32 syscall_mlock(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;}UINT32 syscall_munlock(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;}UINT32 syscall_mlockall(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;}UINT32 syscall_munlockall(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval){	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;}