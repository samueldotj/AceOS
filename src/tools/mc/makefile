#src/tools/mc/makefile

include $(ACE_ROOT)/make.conf

#since we might use different cross compiler for Ace, for testing use the host's default compiler.
CC = /usr/bin/gcc
CFLAGS =  -Wall -Wno-multichar -funsigned-char -fno-leading-underscore -I $(INCLUDE) -gdwarf-2 -g3

TARGET =	$(TOOLS_BIN)/mkmc
OBJS = $(patsubst %.c,%.o,$(wildcard *.c))

#phony - all - make target
all:    $(TARGET)

#phony - clean - clean all object files
clean:
	@rm -f $(OBJS) *.exe

all_clean: clean
	@rm -f $(OBJS:.o=.d)

#how to make target
$(TARGET):	$(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) mkmc.c

#create .d files
-include $(OBJS:.o=.d)
