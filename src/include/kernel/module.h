/*!
  \file		kernel/module.h
  \brief	Contains kernel module container data structure definitions.
*/

#ifndef _MODULE_H_
#define _MODULE_H_

#include <ace.h>
#include <kernel/error.h>

#define KERNEL_MODULE_NAME_MAX	30

#define MODULE_MAGIC_NUMBER 0xACE

struct module_file_header
{
	UINT32 	MagicNumber; 					/*! Identifies Ace kernel module container file - should be 0xACE*/
	int		TotalModules;					/*! Total number of modules present in this container*/
}__attribute__ ((packed));

typedef struct module_file_header MODULE_FILE_HEADER, * MODULE_FILE_HEADER_PTR;

struct module_header
{
	char 	ModuleName[KERNEL_MODULE_NAME_MAX];		/*! Name of the module*/
	UINT32	Size;									/*! Size of the module*/
}__attribute__ ((packed));

typedef struct module_header MODULE_HEADER, * MODULE_HEADER_PTR;

#ifdef __cplusplus
    extern "C" {
#endif

ERROR_CODE InitBootModuleContainer();
ERROR_CODE LoadBootModule(char * module_name, void ** start_address, UINT32 * size);
ERROR_CODE StartBootModule(void * va, UINT32 size);

ERROR_CODE LoadBootModules();

#ifdef __cplusplus
	}
#endif

#endif
