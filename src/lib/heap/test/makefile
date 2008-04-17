#	File			:	lib/heap/test makefile
#   Author			:	Samuel Jacob
#  	Version			:	3.0
#  	Created 		:	09-Apr-2008 8:00PM
#	Last modified	:
#	Brief			:	commands supported are: clean testslab testheap

include $(ACE_ROOT)/make.conf

#since we might use different cross compiler for Ace, for testing use the host's default compiler.
CC = /usr/bin/gcc

CFLAGS =   -W1 -Wno-multichar -funsigned-char -fleading-underscore -I $(INCLUDE)

#phony - all - make target
all:    testslab testheap 

#phony - clean - clean all object files
clean:
	@rm -f *.d *.o
	@rm -f *.exe

#how to make target
testcommon.o:	testcommon.c
	$(CC) $(CFLAGS) -c -o testcommon.o testcommon.c

spinlock.o:	spinlock.c
	$(CC) $(CFLAGS) -c -o spinlock.o spinlock.c
	
testslab:	testcommon.o spinlock.o testslab.c $(OBJ)/libheap.a
	$(CC) $(CFLAGS) -L $(OBJ) -o testslab testslab.c spinlock.o testcommon.o -lheap -lds

testheap:	testcommon.o spinlock.o testheap.c $(OBJ)/libheap.a
	$(CC) $(CFLAGS) -L $(OBJ) -o testheap testheap.c spinlock.o testcommon.o -lheap -lds
	
#create .d files
-include $(OBJS:.o=.d)
