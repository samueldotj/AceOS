#kernel/pic/makefile

include $(ACE_ROOT)/make.conf

TARGET= $(OBJ)/pic.a
OBJS= 	$(patsubst %.c,%.o,$(wildcard *.c))

all:    $(TARGET)

clean: always
	@rm -f *.o *.d

$(TARGET):  $(OBJS)
	@$(AR) ru $(TARGET) $(OBJS)

always:	

#create .d files
-include $(OBJS:.o=.d)
