#	File			:	src/kernel makefile
#	Brief			:	commands supported are: all generic clean 

include $(ACE_ROOT)/make.conf

#clean dirs 
CLEAN_DIRS=	generic rtc pit acpi pic $(ARCH) $(ARCH)/debug $(ARCH)/mm $

#path for the output kernel
TARGET= $(OBJ)/kernel.sys

#libiraries needed to included in kernel linking
LIBS= $(ACE_ROOT)/obj/libheap.a $(ACE_ROOT)/obj/libds.a $(ACE_ROOT)/obj/libstring.a $(ACE_ROOT)/obj/libsync.a

#kernel object files
DEBUG_OBJ = $(patsubst %.c,%.o,$(wildcard $(ARCH)/debug/*.c)) $(patsubst %.asm,%.o,$(wildcard $(ARCH)/debug/*.asm))
KERNEL_OBJ = $(patsubst %.c,%.o,$(wildcard *.c))  $(patsubst %.c,%.o,$(wildcard mm/*.c)) $(patsubst %.c,%.o,$(wildcard pm/*.c))
RTC_OBJ = $(patsubst %.c,%.o,$(wildcard rtc/*.c))
PIT_OBJ = $(patsubst %.c,%.o,$(wildcard pit/*.c))
PIC_OBJ = $(patsubst %.c,%.o,$(wildcard pic/*.c))
ARCH_OBJ = $(patsubst %.c,%.o,$(wildcard $(ARCH)/*.c)) $(patsubst %.asm,%.o,$(wildcard $(ARCH)/*.asm)) $(patsubst %.c,%.o,$(wildcard $(ARCH)/mm/*.c)) $(patsubst %.asm,%.o,$(wildcard $(ARCH)/mm/*.asm)) $(patsubst %.c,%.o,$(wildcard $(ARCH)/pm/*.c)) $(patsubst %.asm,%.o,$(wildcard $(ARCH)/pm/*.asm))
#kernel objects
OBJS= $(KERNEL_OBJ) $(ARCH_OBJ) $(DEBUG_OBJ) $(RTC_OBJ) $(PIT_OBJ) $(PIC_OBJ) $(GENERIC_OBJ) 

#phony command - all - makes all object
all:    generic acpi $(TARGET)

#phony command - generic
generic:	always
	@make -C generic
	
acpi: 	$(OBJ)/acpi.sys
	echo -n "Compiling ACPI ..."
	@make -C acpi
	echo "done"

kernel_build_number:	always
	@$(INCLUDE)/version.sh $(INCLUDE)	
	
#phony commmand - clean - remove all .o, .d in all directories
clean:	always
	@rm -f $(OBJS) $(OBJS:.o=.d)
	@rm	-f *.o *.d  
	@for dir in $(CLEAN_DIRS); do make -C $$dir clean; done
	@rm	-f $(TARGET)

#increment the build number, link the kernel and copy the kernel to image file
$(TARGET):  kernel_build_number $(OBJS)
	@$(LD) -T kernel.ld $(LDFLAGS) -o $(TARGET) $(OBJS) $(OBJ)/acpi.sys $(LIBS) -Map kernel.map
	@echo "New kernel is at $(TARGET)"

#include dependency files
-include $(OBJS:.o=.d)

always:
