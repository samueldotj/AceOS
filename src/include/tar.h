/*! \file 	tar.h
    \brief 	tar file format
*/

#ifndef TAR_H
#define TAR_H

#include <ace.h>
#include <kernel/vfs/vfs.h>

/*POSIXheader.*/
struct tar_posix_header
{						/*byteoffset*/
	char name[100];		/*0*/
	char mode[8];		/*100*/
	char uid[8];		/*108*/
	char gid[8];		/*116*/
	char size[12];		/*124*/
	char mtime[12];		/*136*/
	char chksum[8];		/*148*/
	char typeflag;		/*156*/
	char linkname[100];	/*157*/
	char magic[6];		/*257*/
	char version[2];	/*263*/
	char uname[32];		/*265*/
	char gname[32];		/*297*/
	char devmajor[8];	/*329*/
	char devminor[8];	/*337*/
	char prefix[155];	/*345*/
	char pad[12];
}__attribute__ ((packed));	/*500*/

typedef struct tar_posix_header TAR_POSIX_HEADER, * TAR_POSIX_HEADER_PTR;

#define TMAGIC		"ustar"	/*ustar and a null*/
#define TMAGLEN6
#define TVERSION	"00"	/*00 and no null*/
#define TVERSLEN2

/*Values used in type flag field.*/
#define REGTYPE 	'0'		/*regularfile*/
#define AREGTYPE 	'\0'	/*regularfile*/
#define LNKTYPE		'1'		/*link*/
#define SYMTYPE		'2'		/*reserved*/
#define CHRTYPE		'3'		/*characterspecial*/
#define BLKTYPE		'4'		/*blockspecial*/
#define DIRTYPE		'5'		/*directory*/
#define FIFOTYPE	'6'		/*FIFOspecial*/
#define CONTTYPE	'7'		/*reserved*/

#define XHDTYPE		'x'		/*Extended header referring to the next file in the archive*/
#define XGLTYPE		'g'		/*Global extended header*/

/*Bits used in the mode field, values in octal*/
#define TSUID		04000	/*set UID on execution*/
#define TSGID		02000	/*set GID on execution*/
#define TSVTX		01000	/*reserved*/
/*file permissions*/
#define TUREAD		00400	/*read by owner*/
#define TUWRITE		00200	/*write by owner*/
#define TUEXEC		00100	/*execute/search by owner*/
#define TGREAD		00040	/*read by group*/
#define TGWRITE		00020	/*write by group*/
#define TGEXEC		00010	/*execute/search by group*/
#define TOREAD		00004	/*read by other*/
#define TOWRITE		00002	/*write by other*/
#define TOEXEC		00001	/*execute/search by other*/

#ifdef __cplusplus
    extern "C" {
#endif

int GetDirectoryEntryCountInTar(char * tar_start);
int GetDirectoryEntriesInTar(char * tar_start, char * file_name, int inode, FILE_STAT_PARAM_PTR buffer, int max_entries);

#ifdef __cplusplus
	}
#endif


#endif
/*! \file 	tar.h
    \brief 	tar file format
*/

#ifndef TAR_H
#define TAR_H

#include <ace.h>
#include <kernel/vfs/vfs.h>

/*POSIXheader.*/
struct tar_posix_header
{						/*byteoffset*/
	char name[100];		/*0*/
	char mode[8];		/*100*/
	char uid[8];		/*108*/
	char gid[8];		/*116*/
	char size[12];		/*124*/
	char mtime[12];		/*136*/
	char chksum[8];		/*148*/
	char typeflag;		/*156*/
	char linkname[100];	/*157*/
	char magic[6];		/*257*/
	char version[2];	/*263*/
	char uname[32];		/*265*/
	char gname[32];		/*297*/
	char devmajor[8];	/*329*/
	char devminor[8];	/*337*/
	char prefix[155];	/*345*/
	char pad[12];
}__attribute__ ((packed));	/*500*/

typedef struct tar_posix_header TAR_POSIX_HEADER, * TAR_POSIX_HEADER_PTR;

#define TMAGIC		"ustar"	/*ustar and a null*/
#define TMAGLEN6
#define TVERSION	"00"	/*00 and no null*/
#define TVERSLEN2

/*Values used in type flag field.*/
#define REGTYPE 	'0'		/*regularfile*/
#define AREGTYPE 	'\0'	/*regularfile*/
#define LNKTYPE		'1'		/*link*/
#define SYMTYPE		'2'		/*reserved*/
#define CHRTYPE		'3'		/*characterspecial*/
#define BLKTYPE		'4'		/*blockspecial*/
#define DIRTYPE		'5'		/*directory*/
#define FIFOTYPE	'6'		/*FIFOspecial*/
#define CONTTYPE	'7'		/*reserved*/

#define XHDTYPE		'x'		/*Extended header referring to the next file in the archive*/
#define XGLTYPE		'g'		/*Global extended header*/

/*Bits used in the mode field, values in octal*/
#define TSUID		04000	/*set UID on execution*/
#define TSGID		02000	/*set GID on execution*/
#define TSVTX		01000	/*reserved*/
/*file permissions*/
#define TUREAD		00400	/*read by owner*/
#define TUWRITE		00200	/*write by owner*/
#define TUEXEC		00100	/*execute/search by owner*/
#define TGREAD		00040	/*read by group*/
#define TGWRITE		00020	/*write by group*/
#define TGEXEC		00010	/*execute/search by group*/
#define TOREAD		00004	/*read by other*/
#define TOWRITE		00002	/*write by other*/
#define TOEXEC		00001	/*execute/search by other*/

#ifdef __cplusplus
    extern "C" {
#endif

int GetDirectoryEntryCountInTar(char * tar_start);
int GetDirectoryEntriesInTar(char * tar_start, char * file_name, int inode, FILE_STAT_PARAM_PTR buffer, int max_entries);

#ifdef __cplusplus
	}
#endif


#endif
/*! \file 	tar.h
    \brief 	tar file format
*/

#ifndef TAR_H
#define TAR_H

#include <ace.h>
#include <kernel/vfs/vfs.h>

/*POSIXheader.*/
struct tar_posix_header
{						/*byteoffset*/
	char name[100];		/*0*/
	char mode[8];		/*100*/
	char uid[8];		/*108*/
	char gid[8];		/*116*/
	char size[12];		/*124*/
	char mtime[12];		/*136*/
	char chksum[8];		/*148*/
	char typeflag;		/*156*/
	char linkname[100];	/*157*/
	char magic[6];		/*257*/
	char version[2];	/*263*/
	char uname[32];		/*265*/
	char gname[32];		/*297*/
	char devmajor[8];	/*329*/
	char devminor[8];	/*337*/
	char prefix[155];	/*345*/
	char pad[12];
}__attribute__ ((packed));	/*500*/

typedef struct tar_posix_header TAR_POSIX_HEADER, * TAR_POSIX_HEADER_PTR;

#define TMAGIC		"ustar"	/*ustar and a null*/
#define TMAGLEN6
#define TVERSION	"00"	/*00 and no null*/
#define TVERSLEN2

/*Values used in type flag field.*/
#define REGTYPE 	'0'		/*regularfile*/
#define AREGTYPE 	'\0'	/*regularfile*/
#define LNKTYPE		'1'		/*link*/
#define SYMTYPE		'2'		/*reserved*/
#define CHRTYPE		'3'		/*characterspecial*/
#define BLKTYPE		'4'		/*blockspecial*/
#define DIRTYPE		'5'		/*directory*/
#define FIFOTYPE	'6'		/*FIFOspecial*/
#define CONTTYPE	'7'		/*reserved*/

#define XHDTYPE		'x'		/*Extended header referring to the next file in the archive*/
#define XGLTYPE		'g'		/*Global extended header*/

/*Bits used in the mode field, values in octal*/
#define TSUID		04000	/*set UID on execution*/
#define TSGID		02000	/*set GID on execution*/
#define TSVTX		01000	/*reserved*/
/*file permissions*/
#define TUREAD		00400	/*read by owner*/
#define TUWRITE		00200	/*write by owner*/
#define TUEXEC		00100	/*execute/search by owner*/
#define TGREAD		00040	/*read by group*/
#define TGWRITE		00020	/*write by group*/
#define TGEXEC		00010	/*execute/search by group*/
#define TOREAD		00004	/*read by other*/
#define TOWRITE		00002	/*write by other*/
#define TOEXEC		00001	/*execute/search by other*/

#ifdef __cplusplus
    extern "C" {
#endif

int GetDirectoryEntryCountInTar(char * tar_start);
int GetDirectoryEntriesInTar(char * tar_start, FILE_STAT_PARAM_PTR buffer, int max_entries, int index);
int GetDirectoryEntriesInTarByFileName(char * tar_start, char * file_name, FILE_STAT_PARAM_PTR buffer);

#ifdef __cplusplus
	}
#endif


#endif
