/*!
    \file   uname.c
    \brief  system call related to host name and user ids
*/

#include <ace.h>
#include <string.h>
#include <kernel/debug.h>
#include <libc/sys/utsname.h>
#include <kernel/system_call_handler.h>

/*! get the name of the current system
 * int uname(struct utsname *name);
 * Upon successful completion, a non-negative value shall be returned. Otherwise, -1 shall be returned and errno set to indicate the error.
 * */
UINT32 syscall_uname(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	struct utsname * name;
	
	name = (struct utsname *)sys_call_args->args[0];
	
	strcpy( name->sysname, UTS_SYSTEM_NAME);
	strcpy( name->nodename, UTS_NODE_NAME);
	strcpy( name->release, UTS_RELEASE );
	strcpy( name->version, UTS_VERSION );
	strcpy( name->machine, UTS_MACHINE );

	* retval = 0;
	return 0;
}

/*! get an identifier for the current host*/
UINT32 syscall_gethostid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = 1;
	return 0;
}

/*! get login name 
 * The return value from getlogin() may point to static data whose content is overwritten by each call.
 * */
UINT32 syscall_getlogin(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = 0;
	return 1;
}

/*! If successful, the getlogin_r() function shall return zero; otherwise, an error number shall be returned to indicate the error.
 * [EMFILE] {OPEN_MAX} file descriptors are currently open in the calling process.
 * [ENFILE] The maximum allowable number of files is currently open in the system.
 * [ENXIO] The calling process has no controlling terminal.
 * */
UINT32 syscall_getlogin_r(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = 0;
	return 1;
}

/*! get the current user ID*/
UINT32 syscall_getuid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = 1;
	return 0;
}

/*! get the effective user ID*/
UINT32 syscall_geteuid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = 1;
	return 0;
}

/*! get the group ID*/
UINT32 syscall_getgid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = 1;
	return 0;
}

/*! get the effective group ID*/
UINT32 syscall_getegid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = 1;
	return 0;
}

/*! get the process group ID for a process*/
UINT32 syscall_getpgid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = 1;
	return 0;
}

/*! get the process group ID of the calling process*/
UINT32 syscall_getpgrp(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = 1;
	return 0;
}

/*! get the process ID*/
UINT32 syscall_getpid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = 1;
	return 0;
}

/*! get the parent process ID*/
UINT32 syscall_getppid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = 1;
	return 0;
}

/*! get the process group ID of a session leader*/
UINT32 syscall_getsid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = 1;
	return 0;
}

/*! search user database for a user ID
 * The getpwuid() function shall return a pointer to a struct passwd with the structure as defined in <pwd.h> with a matching entry if found. A null pointer shall be returned if the requested entry is not found, or an error occurs. On error, errno shall be set to indicate the error.
 * */
UINT32 syscall_getpwuid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = NULL;
	return 1;
}

/*!  set user ID
 * The setuid() function shall fail, return -1, and set errno to the corresponding value if one or more of the following are true:
 * [EINVAL] The value of the uid argument is invalid and not supported by the implementation.
 * [EPERM] The process does not have appropriate privileges and uid does not match the real user ID or the saved set-user-ID.
 * */
UINT32 syscall_setuid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = 0;
	return 0;
}

/*! set-group-ID
 * The setgid() function shall fail, return -1, and set errno to the corresponding value if one or more of the following are true:
 * [EINVAL] The value of the uid argument is invalid and not supported by the implementation.
 * [EPERM] The process does not have appropriate privileges and uid does not match the real user ID or the saved set-user-ID.
 * */
UINT32 syscall_setgid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = 0;
	return 0;
}

/*!  set process group ID for job control
 * The setgid() function shall fail, return -1, and set errno to the corresponding value if one or more of the following are true:
 * [EINVAL] The value of the uid argument is invalid and not supported by the implementation.
 * [EPERM] The process does not have appropriate privileges and uid does not match the real user ID or the saved set-user-ID.
 * */
UINT32 syscall_setpgid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = 0;
	return 0;
}

/*!  set the process group ID
 * Upon completion, setpgrp() shall return the process group ID.
 * No errors are defined.
 * */
UINT32 syscall_setpgrp(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = 1;
	return 0;
}

/*! set real and effective group IDs
 * The function shall fail, return -1, and set errno to the corresponding value if one or more of the following are true:
 * [EINVAL] The value of the uid argument is invalid and not supported by the implementation.
 * [EPERM] The process does not have appropriate privileges and uid does not match the real user ID or the saved set-user-ID.
 * */
UINT32 syscall_setregid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	int rgid, egid;
	rgid = sys_call_args->args[0];
	egid = sys_call_args->args[1];
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	
	* retval = 0;
	return 0;
}

/*! set real and effective user IDs
 * The function shall fail, return -1, and set errno to the corresponding value if one or more of the following are true:
 * [EINVAL] The value of the uid argument is invalid and not supported by the implementation.
 * [EPERM] The process does not have appropriate privileges and uid does not match the real user ID or the saved set-user-ID.
 * */
UINT32 syscall_setreuid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	int ruid, euid;
	ruid = sys_call_args->args[0];
	euid = sys_call_args->args[1];
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = 0;
	return 0;
}

/*! create session and set process group ID
 * Upon successful completion, setsid() shall return the value of the new process group ID of the calling process. Otherwise, it shall return (pid_t)-1 and set errno to ndicate the error.
 * [EPERM] The calling process is already a process group leader, or the process group ID of a process other than the calling process matches the process ID of the calling process.*/
UINT32 syscall_setsid(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = 1;
	return 0;	
}
