#	File			:	ds/sync makefile
#   Author			:	Samuel Jacob
#  	Version			:	1.0
#  	Created 		:	19-Apr-2008 11:28
#	Last modified	:
#	Brief			:	commands supported are: all clean 

include $(ACE_ROOT)/make.conf

#since we might use different cross compiler for Ace, for testing use the host's default compiler.
CC = /usr/bin/gcc

#clean dirs 
CLEAN_DIRS=	$(ARCH)

#target path
TARGET= $(OBJ)/libsync.a

#individual objects
GENERIC_OBJ = $(patsubst %.c,%.o,$(wildcard *.c)) $(patsubst %.asm,%.o,$(wildcard *.asm))
ARCH_OBJ = $(patsubst %.c,%.o,$(wildcard $(ARCH)/*.c)) $(patsubst %.asm,%.o,$(wildcard $(ARCH)/*.asm)) 

#all objects for this library
OBJS = $(ARCH_OBJ) $(GENERIC_OBJ)

#phony - all - make target
all:    $(TARGET)

#phony - clean - clean all object files
clean:
	@rm	-f *.o *.d  
	@for dir in $(CLEAN_DIRS); do make -C $$dir clean; done
	@rm	-f $(TARGET)

#how to make target
$(TARGET):  $(OBJS)
	@$(AR) ru $(TARGET) $(OBJS)

#create .d files
-include $(OBJS:.o=.d)
