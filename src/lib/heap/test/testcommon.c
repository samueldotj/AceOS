#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int test_random_free = 0;
int test_first_alloc_last_free = 0;

int verbose_level=1;
int print_stat = 0;

static void print_usage(char * exe)
{
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
					case 'r':
						test_random_free = 1;
						break;
						
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

