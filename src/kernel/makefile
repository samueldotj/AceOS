#src/kernel/makefile

include $(ACE_ROOT)/make.conf

#clean dirs 
CLEAN_DIRS=	rtc pit acpi pic $(ARCH)

#path for the output kernel
TARGET= $(OBJ)/kernel.sys

#libiraries used bykernel
LIBS= $(OBJ)/libheap.a $(OBJ)/libds.a $(OBJ)/libstring.a $(OBJ)/libsync.a

#static kernel modules
STATIC_MODS= $(OBJ)/arch.a $(OBJ)/rtc.a $(OBJ)/pit.a $(OBJ)/pic.a $(OBJ)/acpi.a 

#generica kernel object files
KERNEL_OBJ = $(patsubst %.c,%.o,$(wildcard *.c))  $(patsubst %.c,%.o,$(wildcard mm/*.c)) $(patsubst %.c,%.o,$(wildcard pm/*.c))

#phony command - all - makes all object
all:    $(TARGET)

$(OBJ)/arch.a:	always
	@make -C $(ARCH)

$(OBJ)/rtc.a:	always
	@make -C rtc

$(OBJ)/pit.a:	always
	@make -C pit

$(OBJ)/pic.a:	always
	@make -C pic

$(OBJ)/acpi.a:
	echo -n "Compiling ACPI ..."
	@make -C acpi

kernel_build_number:	always
	@$(INCLUDE)/version.sh $(INCLUDE)	
	
#phony commmand - clean - remove all .o, .d in all directories
clean:	always
	@rm -f $(KERNEL_OBJ) $(KERNEL_OBJ:.o=.d)
	@for dir in $(CLEAN_DIRS); do make -C $$dir clean; done
	@rm -f $(STATIC_MODS)
	@rm	-f $(TARGET)
	

#increment the build number, link the kernel and copy the kernel to image file
$(TARGET):  kernel_build_number $(KERNEL_OBJ) $(STATIC_MODS)
	@$(LD) -T kernel.ld $(LDFLAGS) -Map kernel.map -o $(TARGET) $(KERNEL_OBJ) $(STATIC_MODS)  $(LIBS)
	@echo "New kernel is at $(TARGET)"
	$(ACE_ROOT)/img/create_bootcd.sh

#include dependency files
-include $(KERNEL_OBJ:.o=.d)

always:
