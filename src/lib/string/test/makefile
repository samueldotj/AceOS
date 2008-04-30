#	File			:	lib/string/test makefile
#   Author			:	Samuel Jacob
#  	Version			:	1.0
#  	Created 		:	29-Apr-2008 17:10
#	Last modified	:
#	Brief			:	commands supported are: clean teststring

include $(ACE_ROOT)/make.conf

#since we might use different cross compiler for Ace, for testing use the host's default compiler.
CC = /usr/bin/gcc

CFLAGS =   -Wall -Wno-multichar -funsigned-char -fno-leading-underscore -I $(INCLUDE)

#phony - all - make target
all:    teststring

#phony - clean - clean all object files
clean:
	@rm -f *.d *.o
	@rm -f *.exe

#how to make target
teststring:	teststring.c $(OBJ)/libstring.a
	$(CC) $(CFLAGS) -L $(OBJ) -o teststring teststring.c -lstring


#create .d files
-include $(OBJS:.o=.d)
