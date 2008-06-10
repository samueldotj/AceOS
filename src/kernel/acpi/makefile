#	File			:	kernel/acpi/makefile
#   Author			:	Samuel
#  	Version			:	3.0
#  	Created 		:	06-Jun-2008 11:38
#	Last modified	:	24-Mar-2008 12:19
#	Brief			:	06-Jun-2008 11:38

CUSTOM_FLAG= -I $(ACE_ROOT)/src/include/kernel/acpi -Wno-format

include $(ACE_ROOT)/make.conf

ALL_DIRS= common events hardware interpreter namespace os_specific resources tables

TARGET= $(OBJ)/acpi.sys

OBJS= 	$(patsubst %.c,%.o,$(wildcard *.c)) \
		$(patsubst %.c,%.o,$(wildcard events/*.c)) \
		$(patsubst %.c,%.o,$(wildcard hardware/*.c)) \
		$(patsubst %.c,%.o,$(wildcard interpreter/*.c)) \
		$(patsubst %.c,%.o,$(wildcard namespace/*.c)) \
		$(patsubst %.c,%.o,$(wildcard os_specific/*.c)) \
		$(patsubst %.c,%.o,$(wildcard resources/*.c)) \
		$(patsubst %.c,%.o,$(wildcard tables/*.c)) 

#phony command - all - makes all object
all:    $(TARGET)

clean:	always
	@for dir in $(ALL_DIRS); do cd $$dir; rm -rf *.o *.d; cd ..; done

always:	

$(TARGET):  $(OBJS)
	@$(AR) ru $(TARGET) $(OBJS)

#create .d files
-include $(OBJS:.o=.d)
