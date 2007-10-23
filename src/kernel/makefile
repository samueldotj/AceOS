#	File			:	kernel makefile
#   Author			:	DilipSimha N M and Samuel Jacob
#  	Version			:	3.0
#  	Created 		:	20-Sep-2007 16:14 
#	Last modified	:
#	Brief			:	commands supported are: all generic clean 

include $(ACE_ROOT)/make.conf

#this variable should point to the directory for which the compilation is being done
ARCH=	i386

#clean dirs 
CLEAN_DIRS=	generic $(ARCH) $(ARCH)/debug

#path for the output kernel
TARGET= $(OBJ)/kernel.sys

#libiraries needed to included in kernel linking
LIBS= $(ACE_ROOT)/obj/libstring.a

#kernel object files
DEBUG_OBJ = $(patsubst %.c,%.o,$(wildcard $(ARCH)/debug/*.c)) $(patsubst %.asm,%.o,$(wildcard $(ARCH)/debug/*.asm))
KERNEL_OBJ = $(patsubst %.c,%.o,$(wildcard *.c)) $(patsubst %.c,%.o,$(wildcard $(ARCH)/*.c)) $(patsubst %.asm,%.o,$(wildcard $(ARCH)/*.asm))
#kernel objects
OBJS= $(KERNEL_OBJ) $(DEBUG_OBJ) $(GENERIC_OBJ)

#phony command - all - makes all object
all:    generic $(TARGET)

#phony command - generic
generic:	always
	@make -C generic

#phony commmand - clean - remove all .o, .d in all directories
clean:	always
	@rm	-f *.o *.d  
	@for dir in $(CLEAN_DIRS); do make -C $$dir clean; done
	@rm	-f $(TARGET)

#increment the build number, link the kernel and copy the kernel to image file
$(TARGET):  $(OBJS)
	@$(INCLUDE)/version.sh $(INCLUDE)
	@$(LD) -T kernel.ld $(LDFLAGS) -o $(TARGET) $(OBJS) $(LIBS) -Map kernel.map
	@$(ACE_ROOT)/img/copy_file_to_image.sh $(ACE_ROOT)/img/floppy.ima $(TARGET) .
	@echo -n "Compilation success: New kernel is in $(ACE_ROOT)/img/floppy.ima"

#include dependency files
-include $(OBJS:.o=.d)

always:
