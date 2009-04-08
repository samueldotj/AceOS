#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int verbose_level=1;

void exit(int status);

static void print_usage(char * exe)
{
	printf("Usage : %s /v<n>\n", exe);
}
int parse_arguments(int argc, char * argv[])
{
	//parse arguments
	if ( argc > 1 )
	{
		int i=1;
		while( i < argc )
		{
			if ( argv[i][0] == '/' || argv[i][0] == '-')
			{
				switch( argv[i][1] )
				{
					case 'v':
						verbose_level = argv[i][2]-'0';
						break;
					default:
						print_usage( argv[0] );
						return 1;
				}
				
			}
			else
			{
				print_usage( argv[0] );
				return 1;
			}
			i++;
		}
	}
	
	return 0;
}

void _assert(const char *msg, const char *file, int line)
{
	printf("%s : %s %d", msg, file, line);
	exit(1);
}

