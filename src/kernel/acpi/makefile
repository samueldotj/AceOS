#kernel/acpi/makefile

ifeq ($(ACPI_DEBUG),)
	ACPI_DEBUG=	-g0
endif

CUSTOM_FLAG= -I $(ACE_ROOT)/src/include/kernel/acpi -Wno-format $(ACPI_DEBUG)

include $(ACE_ROOT)/make.conf

ALL_DIRS= . debugger events hardware interpreter/dispatcher interpreter/executer namespace interpreter/parser resources tables utilities

TARGET= $(OBJ)/acpi.a

OBJS= 	$(patsubst %.c,%.o,$(wildcard *.c)) \
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
	rm -f $(OBJS)
	@for dir in $(ALL_DIRS); do rm -rf $$dir/*.o $$dir/*.d; done

always:	

$(TARGET):  $(OBJS)
	@$(AR) ru $(TARGET) $(OBJS)

#create .d files
-include $(OBJS:.o=.d)
