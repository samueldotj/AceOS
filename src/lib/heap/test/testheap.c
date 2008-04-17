#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int parse_arguments(int argc, char * argv[]);

extern int verbose_level;
int main(int argc, char * argv[])
{
	parse_arguments(argc, argv); 
	return 0;
}
