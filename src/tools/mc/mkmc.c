/*! file src/tools/mc/mkmc.c
	Makes module container from the given files
	\todo - add sorting of file name 
*/
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <kernel/module.h>

int verbose = 0;
char * output_file_name = "boot_modules.mod";

/*! aligns a address to page upper boundary*/
#define PAGE_SIZE	4096
#define PAGE_ALIGN_UP(addr)			((UINT32)((addr) + PAGE_SIZE - 1) & -PAGE_SIZE)

void PrintUsage(char * executable)
{
	printf("This utility will create module container for Ace kernel.\n");
	printf("\t Usage : %s [-o <output_file>] [-v] <filename(s)>\n", executable);
	printf("\t Where \n"
			"\t\t -o - to specify output file name else %s will be used\n"
			"\t\t -v - to enable verbose mode\n"
			"\t\t filename(s) - kernel module file names\n", output_file_name);
}

#define EXIT_IF_ERROR( error_code, msg )	\
	{										\
		if ( error_code )					\
		{									\
			perror(msg);					\
			_exit(1);						\
		}									\
	}		

int main(int argc, char * argv[])
{
	int i=1, out_file=0, headers_size=0, input_arg_start, module_count, pad_count=0, module_index=0, content_index=0;
	FILE * in_fp;
	MODULE_FILE_HEADER * file_header = NULL;
	MODULE_HEADER * mod_header = NULL;
	char * mod_content = NULL;
	size_t mmap_size=0;
	
	/*loop through all the options*/
	for(i=1; i<argc; i++)
	{
		/*if not a option break*/
		if ( argv[i][0] != '-' )
			break;
			
		switch( argv[i][1] )
		{
			case 'o':
				if ( i+1 > argc )
				{
					printf("-o requires a following parameter");
					PrintUsage(argv[0]);
					return 1;
				}
				i++;
				output_file_name = argv[i];
				break;
			case 'v':
				verbose ++;
				break;
			default:
				printf("Unknown option %s", argv[i] );
				return 1;
		}
	}
	
	module_count = argc - i;
	input_arg_start = i;
	
	if ( module_count < 1 )
	{
		PrintUsage(argv[0]);
		printf("Module file names not given\n");
		return 1;
	}
	
	if ( verbose )
		printf("Writing %d modules to container : %s\n", module_count, output_file_name );
	
	/*header size*/
	mmap_size = sizeof(MODULE_FILE_HEADER);
	/*loop through all files and mmap size*/
	for(i=input_arg_start; i<input_arg_start+module_count; i++)
	{
		struct stat stat_result;
		EXIT_IF_ERROR ( stat(argv[i], &stat_result) == -1, "stat failed - ");
		mmap_size += sizeof(MODULE_HEADER); /*module header size*/
		mmap_size += stat_result.st_size; /*module content size*/
		/*need padding to cover as page aligned files ?*/
		if ( stat_result.st_size != PAGE_ALIGN_UP(stat_result.st_size) )
		{
			mmap_size += sizeof(MODULE_HEADER); /*pad header size*/
			mmap_size += PAGE_ALIGN_UP(stat_result.st_size)-stat_result.st_size; /*pad size*/
			pad_count++;
		}
	}
	mmap_size  = PAGE_ALIGN_UP(mmap_size);
	
	/*open the file and initialize all contents with 0xf0*/
	out_file = open( output_file_name, O_RDWR | O_CREAT, S_IRWXU|S_IRWXG|S_IROTH|S_IWOTH );
	EXIT_IF_ERROR( out_file == -1, "Output file open error " );
	EXIT_IF_ERROR( lseek(out_file, mmap_size, SEEK_SET) == -1, "lseek " );
	EXIT_IF_ERROR( write (out_file, "\0xf0", 1) != 1, "write ");	
	
	/*mmap the output file*/
	file_header = mmap(0, mmap_size, PROT_READ|PROT_WRITE, MAP_SHARED, out_file, 0);
	EXIT_IF_ERROR( file_header == MAP_FAILED, "mmap " );

	/*write file header contents*/
	file_header->MagicNumber = MODULE_MAGIC_NUMBER;
	file_header->TotalModules = module_count+pad_count+1;/*actual modules + pad for modules + pad for file header*/
	
	/*starting address of module headers*/
	mod_header = (MODULE_HEADER_PTR)(((char *)file_header) + sizeof(MODULE_FILE_HEADER));
	/*starting address of module contents*/
	mod_content = ((char *)mod_header) + (sizeof(MODULE_HEADER)*file_header->TotalModules);
	headers_size = sizeof(MODULE_FILE_HEADER) + (sizeof(MODULE_HEADER)*file_header->TotalModules);
	if( headers_size != PAGE_ALIGN_UP(headers_size) )
	{
		strcpy(mod_header[module_index].ModuleName, "headers_pad");
		mod_header[module_index].Size = PAGE_ALIGN_UP(headers_size) - headers_size;
		content_index += mod_header[module_index].Size;
		module_index++;
	}
	/*loop through all files and add them to container*/
	for(i=input_arg_start; i<input_arg_start+module_count; i++)
	{
		char * module_name, * input_filepath = argv[i];
		size_t file_size = 0, pad_size;

		/*select module name from file name by removing the slashes*/
		module_name = strrchr( input_filepath, '//');
		if ( module_name )
			module_name++;
		else
			module_name = input_filepath;
		if ( strlen(module_name)+1 > KERNEL_MODULE_NAME_MAX )
		{
			printf("Module name is greater than %d", KERNEL_MODULE_NAME_MAX );
			return 1;
		}

		/*copy the module contents*/
		in_fp = fopen( input_filepath, "rb" );
		EXIT_IF_ERROR( in_fp == NULL, "input file open error ");
		while( !feof(in_fp) )
		{
			file_size ++;
			mod_content[content_index++] = fgetc(in_fp);
		}
		fclose(in_fp);
		
		/*update the module headers*/
		strcpy(mod_header[module_index].ModuleName, module_name);
		mod_header[module_index].Size = file_size;
		
		if ( verbose )
			printf("Wrote module %s %d\n", module_name,  file_size);
		
		module_index++;
		
		/*need padding to cover as page aligned files?*/
		pad_size = PAGE_ALIGN_UP( file_size ) - file_size;
		if ( pad_size )
		{
			strcpy(mod_header[module_index].ModuleName, "pad");
			mod_header[module_index].Size = pad_size;
			
			content_index += pad_size;
			module_index++;
			if ( verbose )
				printf("Wrote pad %d\n", pad_size);
		}
	}
	
	/*fill up the empty slots*/
	while( module_index< file_header->TotalModules )
	{
		strcpy(mod_header[module_index].ModuleName, "empty");
		mod_header[module_index++].Size = 0;
	}
	
	/*finish*/
	munmap( file_header, mmap_size );
	close(out_file);
	
	return 0;
}
