/*!
  \file		src/kernel/boot_module.c
  \brief	Boot time kernel module load routines
*/

#include <ace.h>
#include <stdlib.h>
#include <string.h>
#include <kernel/module.h>
#include <kernel/debug.h>
#include <kernel/error.h>
#include <kernel/mm/vm.h>

#define BOOT_MODULE_LIST_FILE	"boot_modules_list"

static MODULE_HEADER_PTR boot_module_header_start = NULL;
static int total_modules = 0;

/*! load and initalize the boot module container
	\param file_header - starting virtual address of the module container
*/
ERROR_CODE InitBootModuleContainer()
{
	MODULE_FILE_HEADER_PTR file_header = (MODULE_FILE_HEADER_PTR) kernel_reserve_range.module_va_start;
	if ( file_header == NULL || file_header->MagicNumber != MODULE_MAGIC_NUMBER )
		return ERROR_INVALID_FORMAT;
	
	boot_module_header_start = (MODULE_HEADER_PTR) ( ((char *)file_header) + sizeof(MODULE_FILE_HEADER) );
	total_modules = file_header->TotalModules;
	
	return ERROR_SUCCESS;
}

/*! Load a given the kernel module into memory and return its loaded address
	\param module_name - name of the module to load
	\param start_address - pointer to store the loaded address
	\param size - pointer to store the size of the module
*/
ERROR_CODE LoadBootModule(char * module_name, void ** start_address, UINT32 * size)
{
	MODULE_HEADER_PTR module_header = boot_module_header_start;
	char * module_content_start;
	int i = 0;
	
	/*module content starts after all the headers*/
	module_content_start = ((char *)boot_module_header_start) + ( sizeof(MODULE_HEADER) * total_modules );
		
	assert( start_address != NULL );
	*start_address = NULL;
	while (i<total_modules)
	{
		/*! check the module name*/
		if ( strcmp(module_name, module_header->ModuleName ) == 0 )
		{
			//kprintf("%s %d @%p\n", module_name, module_header->Size, module_content_start);
			/*! update the result and return success*/
			*start_address = (void *)module_content_start;
			if ( size )
				*size = module_header->Size;
			return ERROR_SUCCESS;
		}
		i++;
		module_content_start += module_header->Size;
		/*next module*/
		module_header++;
	}
	return ERROR_NOT_FOUND;
}

/*! Start a already boot module
	\param va - virtual address where the kernel module is loaded
	\param size - size of the kernel module
*/
ERROR_CODE StartBootModule(void * va, UINT32 size)
{
	return ERROR_SUCCESS;
}

