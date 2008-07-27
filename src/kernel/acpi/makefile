#	File			:	kernel/acpi/makefile
#   Author			:	Samuel
#  	Version			:	3.0
#  	Created 		:	06-Jun-2008 11:38
#	Last modified	:	24-Mar-2008 12:19
#	Brief			:	06-Jun-2008 11:38

CUSTOM_FLAG= -I $(ACE_ROOT)/src/include/kernel/acpi -Wno-format

include $(ACE_ROOT)/make.conf

ALL_DIRS= debugger events hardware interpreter/dispatcher interpreter/executer namespace interpreter/parser resources tables utilities

TARGET= $(OBJ)/acpi.sys

OBJS= 	$(patsubst %.c,%.o,$(wildcard *.c)) \
		$(patsubst %.c,%.o,$(wildcard debugger/*.c)) \
		$(patsubst %.c,%.o,$(wildcard events/*.c)) \
		$(patsubst %.c,%.o,$(wildcard hardware/*.c)) \
		$(patsubst %.c,%.o,$(wildcard interpreter/dispatcher/*.c)) \
		$(patsubst %.c,%.o,$(wildcard interpreter/executer/*.c)) \
		$(patsubst %.c,%.o,$(wildcard interpreter/parser/*.c)) \
		$(patsubst %.c,%.o,$(wildcard namespace/*.c)) \
		$(patsubst %.c,%.o,$(wildcard resources/*.c)) \
		$(patsubst %.c,%.o,$(wildcard tables/*.c)) \
		$(patsubst %.c,%.o,$(wildcard utilities/*.c)) 

#phony command - all - makes all object
all:    $(TARGET)

clean:	always
	@for dir in $(ALL_DIRS); do rm -rf $$dir/*.o *.d; done

always:	

$(TARGET):  $(OBJS)
	@$(AR) ru $(TARGET) $(OBJS)

#create .d files
-include $(OBJS:.o=.d)
