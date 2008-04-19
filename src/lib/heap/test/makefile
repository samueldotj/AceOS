#	File			:	lib/heap/test makefile
#   Author			:	Samuel Jacob
#  	Version			:	3.0
#  	Created 		:	09-Apr-2008 8:00PM
#	Last modified	:
#	Brief			:	commands supported are: clean testslab testheap

include $(ACE_ROOT)/make.conf

#since we might use different cross compiler for Ace, for testing use the host's default compiler.
CC = /usr/bin/gcc

CFLAGS =   -Wall -Wno-multichar -funsigned-char -fno-leading-underscore -I $(INCLUDE)

#phony - all - make target
all:    testslab testheap 

#phony - clean - clean all object files
clean:
	@rm -f *.d *.o
	@rm -f *.exe

#how to make target
testcommon.o:	testcommon.c
	$(CC) $(CFLAGS) -c -o testcommon.o testcommon.c

testslab:	testcommon.o testslab.c $(OBJ)/libheap.a
	$(CC) $(CFLAGS) -L $(OBJ) -o testslab testslab.c testcommon.o -lheap -lds -lsync

testheap:	testcommon.o testheap.c $(OBJ)/libheap.a
	$(CC) $(CFLAGS) -L $(OBJ) -o testheap testheap.c testcommon.o -lheap -lds -lsync
	
#create .d files
-include $(OBJS:.o=.d)
