#lib/ds/makefile

include $(ACE_ROOT)/make.conf

#since we might use different cross compiler for Ace, for testing use the host's default compiler.
#CC = /usr/bin/gcc

#objects for this library
OBJS = $(patsubst %.c,%.o,$(wildcard *.c))

#target path
TARGET= $(OBJ)/libds.a

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
