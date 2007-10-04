#	File			:	generic/strig makefile
#   Author			:	Samuel Jacob
#  	Version			:	3.0
#  	Created 		:	20-Sep-2007 18:23
#	Last modified	:
#	Brief			:	commands supported are: all clean 

include $(ACE_ROOT)/make.conf

TARGET= $(OBJ)/libstring.a

OBJS = $(patsubst %.c,%.o,$(wildcard *.c)) $(patsubst %.asm,%.o,$(wildcard *.asm))

all:    $(TARGET)

$(TARGET):  $(OBJS)
	$(AR) ru $(TARGET) $(OBJS)

clean:
	rm	$(OBJS)
	rm	$(TARGET)
	rm	$(OBJS:.o=.d)

include $(OBJS:.o=.d)
