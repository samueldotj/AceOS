#kernel/pit/makefile

include $(ACE_ROOT)/make.conf

TARGET= $(OBJ)/pit.a
OBJS= 	$(patsubst %.c,%.o,$(wildcard *.c))

all:    $(TARGET)

clean: always
	@rm -f $(OBJS)

#phony - clean - clean including .d files
all_clean:	clean
	@rm -f $(OBJS:.o=.d)


$(TARGET):  $(OBJS)
	@$(AR) ru $(TARGET) $(OBJS)

always:	

#create .d files
-include $(OBJS:.o=.d)
