/*!
    \file   file.c
    \brief  system call  for handling files
*/

#include <ace.h>
#include <string.h>
#include <libc/sys/errno.h>
#include <libc/sys/fcntl.h>
#include <libc/sys/stat.h>
#include <kernel/debug.h>
#include <kernel/system_call_handler.h>
#include <kernel/pm/task.h>
#include <kernel/vfs/vfs.h>

/*! open a file
 * int open(const char *path, int oflag)
 * Upon successful completion, the function shall open the file and return a non-negative integer representing the lowest numbered unused file descriptor. 
 * Otherwise, -1 shall be returned and errno set to indicate the error. No files shall be created or modified if the function returns -1.
 * */
UINT32 syscall_open(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	char *path;
	UINT32 oflag;
	VFS_ACCESS_TYPE access=0;
	VFS_OPEN_FLAG flag=0;
	ERROR_CODE ret;
	
	/*\todo - convert relative path to absolute path */
	path = (char *)sys_call_args->args[0];
	/*\todo - convert user mode flag to VFS understanable flag*/
	oflag = sys_call_args->args[1];
	if( oflag & O_RDONLY )
		access = VFS_ACCESS_TYPE_READ;
	else if( oflag & O_RDONLY )
		access = VFS_ACCESS_TYPE_WRITE;
	else if( oflag & O_RDWR )
		access = VFS_ACCESS_TYPE_WRITE;
	
	KTRACE("%s %x\n", path, oflag);
	ret = OpenFile( GetCurrentTask(), path, access, flag, (int *)retval );
	KTRACE("%s\n", ERROR_CODE_AS_STRING(ret));
	if ( ret == ERROR_SUCCESS )
		return 0;
	else
		return EPERM;
}

/*! close a file descriptor
 * Upon successful completion, 0 shall be returned; otherwise, -1 shall be returned and errno set to indicate the error.
 * [EBADF] - The fildes argument is not a valid file descriptor.
 * [EINTR] - The close() function was interrupted by a signal.
 * [EIO] - An I/O error occurred while reading from or writing to the file system.
 * */
UINT32 syscall_close(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	CloseFile( GetCurrentTask(), sys_call_args->args[0] );
	* retval = 0;
	return 0;
}

UINT32 syscall_read(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("(file=%d buf=%p size=%d)\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2]);
	int size;
	char * buf;
	ERROR_CODE err;
	
	size = sys_call_args->args[2];
	buf = (char *)sys_call_args->args[1];
	err = ReadWriteFile( sys_call_args->args[0], size, buf, 0, retval);
	KTRACE("%s\n", ERROR_CODE_AS_STRING(err));
	
	return 0;
}
UINT32 syscall_write(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("(file=%d buf=%p size=%d)\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2]);
	int i, size;
	char * buf;
	
	i = 0;
	size = sys_call_args->args[2];
	buf = (char *)sys_call_args->args[1];
	while(i<size && buf)
		kprintf("%c", buf[i++]);
	*retval = i;
	return 0;
}

UINT32 syscall_link(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_unlink(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_chdir(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
/*! get the pathname of the current working directory
 * char *getcwd(char *buf, size_t size);
 * [EINVAL] - The size argument is 0.
 * [ERANGE] - The size argument is greater than 0, but is smaller than the length of the pathname +1.
 * [EACCES] - Read or search permission was denied for a component of the pathname.
 * [ENOMEM] - Insufficient storage space is available.*/
UINT32 syscall_getcwd(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %d\n", sys_call_args->args[0], sys_call_args->args[1]);
	//strcpy( sys_call_args->args[0], "/test_getcwd" );
	* retval = NULL;// sys_call_args->args[0];
	return 1;
}
UINT32 syscall_mknod(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_chmod(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_fchmod(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
/*  get file status 
 * int stat(const char *restrict path, struct stat *restrict buf);
 * obtain information about the named file and write it to the area pointed to by the buf argument. The path argument points to a pathname naming a file.
 * 
 * Upon successful completion, 0 shall be returned. Otherwise, -1 shall be returned and errno set to indicate the error.
 * */
UINT32 syscall_stat(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%s %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_lstat(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_lseek(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}

/*! int fstat(int fildes, struct stat *buf) - get file status
 * The fstat() function shall obtain information about an open file associated with the file descriptor fildes, and shall write it to the area pointed to by buf.
 * Upon successful completion, 0 shall be returned. Otherwise, -1 shall be returned and errno set to indicate the error.
 * [EBADF] - The fildes argument is not a valid file descriptor.
 * [EIO] - An I/O error occurred while reading from the file system.
 * [EOVERFLOW] - The file size in bytes or the number of blocks allocated to the file or the file serial number cannot be represented correctly in the structure pointed to by buf.
 * [EOVERFLOW] - One of the values is too large to store into the structure pointed to by the buf argument.
 * */
UINT32 syscall_fstat(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_statfs64(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_fstatfs64(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_sync(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_rename(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_mkdir(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_rmdir(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_fchdir(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_select(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_poll(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_dup(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_dup2(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_pipe(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_ioctl(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_fcntl(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_open_dir(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_chroot(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_close_dir(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_read_dir(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_read_dir_r(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_rewind_dir(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_seek_dir(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_tell_dir(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_truncate(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
UINT32 syscall_ftruncate(SYSTEM_CALL_ARGS_PTR sys_call_args, UINT32 *retval)
{
	KTRACE("%p %p %p %p\n", sys_call_args->args[0], sys_call_args->args[1], sys_call_args->args[2], sys_call_args->args[3]);
	* retval = -1;
	return 0;
}
