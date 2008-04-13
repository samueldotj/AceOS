#	File			:	lib/heap makefile
#   Author			:	DilipSimha
#  	Version			:	3.0
#  	Created 		:	24-Mar-2008 12:16
#	Last modified	:	24-Mar-2008 12:19
#	Brief			:	commands supported are: all clean 

include $(ACE_ROOT)/make.conf

#since we might use different cross compiler for Ace, for testing use the host's default compiler.
CC = /usr/bin/gcc

#objects for this library
OBJS = $(patsubst %.c,%.o,$(wildcard *.c)) $(patsubst %.asm,%.o,$(wildcard *.asm))

#target path
TARGET= $(OBJ)/libheap.a

#phony - all - make target
all:    $(TARGET)

#phony - clean - clean all object files
clean:
	@rm -f *.d *.o
	@rm -f $(TARGET)

#how to make target
$(TARGET):  $(OBJS)
	@$(AR) ru $(TARGET) $(OBJS)

#create .d files
-include $(OBJS:.o=.d)
