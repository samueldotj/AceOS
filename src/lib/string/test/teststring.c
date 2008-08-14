#include <assert.h>
#include <stdio.h>
#include <string.h>
void exit(int status);

void _assert(const char *msg, const char *file, int line)
{
	printf("%s : %s %d", msg, file, line);
	exit(1);
}

#define ARRAY_COUNT 	20
int main(int argc, char* argv[])
{
	int i, count;
	char * num[]={
		"1234",
		"0x1234",
		"01234",
		"0xFF",
		"0xff",
		"0xfF",
		"0x3f8",
		"0xF00",
		"-1234",
		"-1",
		"12ab",
		"0b101011",
		"0b10",
		"0b1",
		"0B11"
		};
	
	count = sizeof(num)/sizeof(num[0]);
	
	printf("strtol() :: \n");
	for(i=0;i<count;i++)
	{
		long val = strtol(num[i], NULL, 0);
		printf( "%+10s %20ld %#20lx %#20lo\n", num[i], val, val, val);
	}
	printf("strtoul() :: \n");
	for(i=0;i<count;i++)
	{
		unsigned long val = strtoul(num[i], NULL, 0);
		printf( "%+10s %20uld %#20ulx %#20ulo\n", num[i], val, val, val);
	}
	
	return 0;
}

