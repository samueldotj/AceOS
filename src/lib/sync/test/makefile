#	File			:	lib/sync/test makefile
#   Author			:	Samuel Jacob
#  	Version			:	3.0
#  	Created 		:	22-Apr-2008 12:30
#	Last modified	:
#	Brief			:	commands supported are: clean testspin

include $(ACE_ROOT)/make.conf

#since we might use different cross compiler for Ace, for testing use the host's default compiler.
CC = /usr/bin/gcc

CFLAGS =   -Wall -Wno-multichar -funsigned-char -fno-leading-underscore -I $(INCLUDE)

#phony - all - make target
all:    testspin

#phony - clean - clean all object files
clean:
	@rm -f *.d *.o
	@rm -f *.exe

#how to make target
testcommon.o:	testcommon.c
	$(CC) $(CFLAGS) -c -o testcommon.o testcommon.c
	
testspin:	testcommon.o testspin.c $(OBJ)/libsync.a
	$(CC) $(CFLAGS) -L $(OBJ) -o testspin testspin.c testcommon.o -lsync


#create .d files
-include $(OBJS:.o=.d)
