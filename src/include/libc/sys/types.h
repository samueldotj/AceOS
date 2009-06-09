#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

#if !defined(__time_t_defined) && !defined(_TIME_T)
#define _TIME_T
#define __time_t_defined
typedef _TIME_T_ time_t;
#endif

#if !defined(__clock_t_defined) && !defined(_CLOCK_T)
#define _CLOCK_T
#define __clock_t_defined
typedef _CLOCK_T_ clock_t;
#endif

#ifndef _SSIZE_T
#define _SSIZE_T
typedef _ssize_t ssize_t;
#endif

#ifndef __u_char_defined
#ifdef __GNUC__
__extension__ typedef long long quad_t;
__extension__ typedef unsigned long long u_quad_t;
#else
typedef struct
{
	long int __val[2];
} quad_t;
typedef struct
{
	unsigned long __val[2];
} u_quad_t;
#endif
typedef struct
{
	int __val[2];
} fsid_t;
#define __u_char_defined
#endif

typedef int clockid_t;

#  define _SYS_TYPES_FD_SET
#  define	NBBY	8		/* number of bits in a byte */
/*
 * Select uses bit masks of file descriptors in longs.
 * These macros manipulate such bit fields (the filesystem macros use chars).
 * FD_SETSIZE may be defined by the user, but the default here
 * should be >= NOFILE (param.h).
 */
#  ifndef	FD_SETSIZE
#	define	FD_SETSIZE	64
#  endif

typedef	long	fd_mask;
#  define	NFDBITS	(sizeof (fd_mask) * NBBY)	/* bits per mask */
#  ifndef	howmany
#	define	howmany(x,y)	(((x)+((y)-1))/(y))
#  endif

typedef struct {
        unsigned long fds_bits [(1024/(8 * sizeof(unsigned long)))];
} __fd_set;

typedef __fd_set fd_set;

#  define	FD_SET(n, p)	((p)->fds_bits[(n)/NFDBITS] |= (1L << ((n) % NFDBITS)))
#  define	FD_CLR(n, p)	((p)->fds_bits[(n)/NFDBITS] &= ~(1L << ((n) % NFDBITS)))
#  define	FD_ISSET(n, p)	((p)->fds_bits[(n)/NFDBITS] & (1L << ((n) % NFDBITS)))
#  define	FD_ZERO(p)	(__extension__ (void)({ \
     size_t __i; \
     char *__tmp = (char *)p; \
     for (__i = 0; __i < sizeof (*(p)); ++__i) \
       *__tmp++ = 0; \
}))

#define __mode_t_defined
#define __gid_t_defined
#define __uid_t_defined
#define __pid_t_defined
#define __ssize_t_defined
#define __key_t_defined
#define __off_t_defined
#define __off64_t_defined

typedef int dev_t;
typedef int ino_t;
typedef int mode_t;
typedef int caddr_t;
typedef long off_t;
typedef int pid_t;
typedef int uid_t;
typedef int gid_t;
typedef int key_t;
typedef int nlink_t;
typedef long suseconds_t;
typedef long useconds_t;

typedef long blksize_t;
typedef long blkcnt_t;

typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;
typedef unsigned char u_int8_t;
typedef unsigned short u_int16_t;
typedef unsigned long u_int32_t;
typedef unsigned long long u_int64_t;

typedef u_int32_t __uint32_t;
typedef int32_t	__int32_t;
typedef u_int16_t	__uint16_t;
typedef int16_t	__int16_t;

#endif
