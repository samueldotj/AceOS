#ifndef _SYS_UTSNAME_H
#define _SYS_UTSNAME_H

#include <ace.h>

#define UTS_MAX_NAME	50		

#define UTS_SYSTEM_NAME		"Ace"
#define UTS_NODE_NAME		"Ace Node"
#define UTS_RELEASE			"I"
#define UTS_VERSION			"3"
#define	UTS_MACHINE			"ARCH"


struct utsname {
    char sysname[UTS_MAX_NAME];			/*! name of this implementation of the operating system*/
    char nodename[UTS_MAX_NAME];		/*! name of this node within an implementation-dependent communications network*/
    char release[UTS_MAX_NAME];			/*! current release level of this implementation*/
    char version[UTS_MAX_NAME];			/*! current version level of this release*/
    char machine[UTS_MAX_NAME];			/*! name of the hardware type on which the system is running*/
};

#endif
