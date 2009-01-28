#src/drivers/pci/makefile

include $(ACE_ROOT)/make.conf

TARGET= $(OBJ)/pci_bus.sys
OBJS= 	$(patsubst %.c,%.o, $(wildcard *.c))

#perform partial linking
LD_FLAGS= -r

all:    $(TARGET)

clean: always
	@rm -f $(TARGET)
	@rm -f *.o *.d

$(TARGET):  $(OBJS)
	@$(LD) $(LD_FLAGS) $(OBJS) -o $(TARGET)

always:	

#create .d files
-include $(OBJS:.o=.d)
