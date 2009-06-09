#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#include <sys/types.h>

struct stat
{
	dev_t     st_dev;		/*Device ID of device containing file. */
	ino_t     st_ino;		/*File serial number. */
	mode_t    st_mode;		/*Mode of file */
	nlink_t   st_nlink;		/*Number of hard links to the file. */
	uid_t     st_uid;		/*User ID of file. */
	gid_t     st_gid;		/*Group ID of file. */
	
	dev_t     st_rdev;		/*Device ID (if file is character or block special). */

	off_t     st_size;		/*For regular files, the file size in bytes.  
							For symbolic links, the length in bytes of the pathname contained in the symbolic link.
							For a shared memory object, the length in bytes. 
							For a typed memory object, the length in bytes. 
							For other file types, the use of this field is unspecified. */
	time_t    st_atime;		/*Time of last access. */
	time_t    st_mtime;		/*Time of last data modification. */
	time_t    st_ctime;		/*Time of last status change. */
	
	blksize_t st_blksize;	/*A file system-specific preferred I/O block size for this object. In some file system types, this may vary from file to file.*/
	blkcnt_t  st_blocks;	/*Number of blocks allocated for this object.*/
};

/* Traditional mask definitions for st_mode. */
#define S_IFMT  0170000 /* type of file */
#define S_IFLNK 0120000 /* symbolic link */
#define S_IFREG 0100000 /* regular */
#define S_IFBLK 0060000 /* block special */
#define S_IFDIR 0040000 /* directory */
#define S_IFCHR 0020000 /* character special */
#define S_IFIFO 0010000 /* this is a FIFO */
#define S_ISUID 0004000 /* set user id on execution */
#define S_ISGID 0002000 /* set group id on execution */
                                /* next is reserved for future use */
#define S_ISVTX   01000         /* save swapped text even after use */

/* POSIX masks for st_mode. */
#define S_IRWXU   00700         /* owner:  rwx------ */
#define S_IRUSR   00400         /* owner:  r-------- */
#define S_IWUSR   00200         /* owner:  -w------- */
#define S_IXUSR   00100         /* owner:  --x------ */

#define S_IRWXG   00070         /* group:  ---rwx--- */
#define S_IRGRP   00040         /* group:  ---r----- */
#define S_IWGRP   00020         /* group:  ----w---- */
#define S_IXGRP   00010         /* group:  -----x--- */

#define S_IRWXO   00007         /* others: ------rwx */
#define S_IROTH   00004         /* others: ------r-- */ 
#define S_IWOTH   00002         /* others: -------w- */
#define S_IXOTH   00001         /* others: --------x */

/* Synonyms for above. */
#define S_IEXEC         S_IXUSR
#define S_IWRITE        S_IWUSR
#define S_IREAD         S_IRUSR

/* The following macros test st_mode (from POSIX Sec. 5.6.1.1). */
#define S_ISREG(m)      (((m) & S_IFMT) == S_IFREG)     /* is a reg file */
#define S_ISDIR(m)      (((m) & S_IFMT) == S_IFDIR)     /* is a directory */
#define S_ISCHR(m)      (((m) & S_IFMT) == S_IFCHR)     /* is a char spec */
#define S_ISBLK(m)      (((m) & S_IFMT) == S_IFBLK)     /* is a block spec */
#define S_ISLNK(m)      (((m) & S_IFMT) == S_IFLNK)     /* is a symlink */
#define S_ISFIFO(m)     (((m) & S_IFMT) == S_IFIFO)     /* is a pipe/FIFO */

#endif
