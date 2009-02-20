#kernel/iom/makefile

include $(ACE_ROOT)/make.conf

TARGET= $(OBJ)/iom.a
OBJS= 	$(patsubst %.c,%.o,$(wildcard *.c))

all:    $(TARGET)

clean: always
	@rm -f *.o 

#phony - clean - clean including .d files
all_clean:	clean
	@rm -f *.d 

$(TARGET):  $(OBJS)
	@$(AR) ru $(TARGET) $(OBJS)

always:	

#create .d files
-include $(OBJS:.o=.d)
