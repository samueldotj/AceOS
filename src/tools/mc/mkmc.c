/*! file src/tools/mc/mkmc.c
	Makes module container from the given files
	\todo - add sorting of file name 
*/
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <kernel/module.h>

int verbose = 0;
char * output_file_name = "boot_modules.mod";

void PrintUsage(char * executable)
{
	printf("This utility will create module container for Ace kernel.\n");
	printf("\t Usage : %s [-o <output_file>] [-v] <filename(s)>\n", executable);
	printf("\t Where \n"
			"\t\t -o - to specify output file name else %s will be used\n"
			"\t\t -v - to enable verbose mode\n"
			"\t\t filename(s) - kernel module file names\n", output_file_name);
}
int main(int argc, char * argv[])
{
	int i=1, input_arg_start, module_count;
	FILE * out_fp;
	FILE * in_fp;
	MODULE_FILE_HEADER file_header;
	
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
					return 2;
				}
				i++;
				output_file_name = argv[i];
				break;
			case 'v':
				verbose ++;
				break;
			default:
				printf("Unknown option %s", argv[i] );
				return 2;
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
		printf("Writing %d modules to output file : %s\n", module_count, output_file_name );
	
	out_fp = fopen( output_file_name, "wb" );
	if ( out_fp == NULL )
	{
		perror( "Output open error - ");
		return 3;
	}
	
	file_header.MagicNumber = MODULE_MAGIC_NUMBER;
	file_header.TotalModules = module_count;
	
	/*write file header*/
	fwrite( &file_header, sizeof(file_header), 1, out_fp );
	
	/*loop through all files and add module header for each file*/
	for(i=input_arg_start; i<input_arg_start+module_count; i++)
	{
		struct stat stat_result;
		MODULE_HEADER mod_header;
		char * input_filepath = argv[i];
		char * module_name;

		if ( stat(input_filepath, &stat_result) == -1 )
		{
			perror("stat failed - ");
			return 4;
		}
		
		module_name = strrchr( input_filepath, '//');
		if ( module_name )
			module_name++;
		else
			module_name = input_filepath;
		if ( strlen(module_name)+1 > KERNEL_MODULE_NAME_MAX )
		{
			printf("Module name is greater than %d", KERNEL_MODULE_NAME_MAX );
			return 5;
		}
		
		strcpy(mod_header.ModuleName, module_name);
		mod_header.Size = stat_result.st_size;
		
		if ( verbose )
			printf("Generating module header for %s %d\n", module_name,  (int)mod_header.Size);
		
		/*write module header*/
		fwrite( &mod_header, sizeof(mod_header), 1, out_fp );
	}
	
	/*loop through all files and add them to container*/
	for(i=input_arg_start; i<input_arg_start+module_count; i++)
	{
		char * input_filepath = argv[i];
		in_fp = fopen( input_filepath, "rb" );
		
		if ( in_fp == NULL )
		{
			printf("%s ", input_filepath);
			perror( "Input file open/read error - ");
			return 3;
		}
		/*copy the contents*/
		while( !feof(in_fp) )
			fputc( fgetc(in_fp), out_fp );
			
		fclose(in_fp);
	}
	
	fclose(out_fp);
	
	return 0;
}
