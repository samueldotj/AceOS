#	File			:	src/tools/mc/makefile
#	Brief			:	commands supported are: clean mkmc

include $(ACE_ROOT)/make.conf

#since we might use different cross compiler for Ace, for testing use the host's default compiler.
CC = /usr/bin/gcc

CFLAGS =   -Wall -Wno-multichar -funsigned-char -fno-leading-underscore -I $(INCLUDE)

#phony - all - make target
all:    $(ACE_ROOT)/obj/mkmc

#phony - clean - clean all object files
clean:
	@rm -f *.d *.o
	@rm -f *.exe

#how to make target
$(ACE_ROOT)/obj/mkmc:	mkmc.c
	$(CC) $(CFLAGS) -o $(ACE_ROOT)/obj/mkmc mkmc.c

#create .d files
-include $(OBJS:.o=.d)
