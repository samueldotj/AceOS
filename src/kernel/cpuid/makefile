#	File			:	kernel/acpi/makefile
#   Author			:	DilipSimha N M
#  	Version			:	3.0
#  	Created 		:	16-Jun-2008 16:33
#	Last modified	:	16-Jun-2008 16:33
#	Brief			:	makefile

CUSTOM_FLAG= -I $(ACE_ROOT)/src/include/kernel/ -Wno-format

include $(ACE_ROOT)/make.conf

TARGET= $(OBJ)/cpuid.a

OBJS= 	$(patsubst %.c,%.o,$(wildcard *.c))

#phony command - all - makes all object
all:    $(TARGET)

clean:	always
	rm -rf *.o *.d;
	@rm -f $(TARGET)

always:	

$(TARGET):  $(OBJS)
	@$(AR) ru $(TARGET) $(OBJS)

#create .d files
-include $(OBJS:.o=.d)
