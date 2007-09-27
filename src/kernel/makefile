#	File			:	kernel makefile
#   Author			:	DilipSimha N M and Samuel Jacob
#  	Version			:	3.0
#  	Created 		:	20-Sep-2007 16:14 
#	Last modified	:
#	Brief			:	commands supported are: kernel all clean 

include $(ACE_ROOT)/make.conf

ARCH=	i386
TARGET= $(OBJ)/kernel.sys

KERNEL_OBJ = $(patsubst %.c,%.o,$(wildcard *.c)) $(patsubst %.c,%.o,$(wildcard $(ARCH)/*.c)) $(patsubst %.asm,%.o,$(wildcard $(ARCH)/*.asm))
DEBUG_OBJ = $(patsubst %.c,%.o,$(wildcard $(ARCH)/debug/*.c))

OBJS= $(KERNEL_OBJ) $(DEBUG_OBJ) $(GENERIC_OBJ)

LIBS= $(ACE_ROOT)/obj/libstring.a

all:    generic $(TARGET)

$(TARGET):  $(OBJS)
	$(LD) -T kernel.ld $(LDFLAGS) -o $(TARGET) $(OBJS) $(LIBS) -Map kernel.map 
	$(ACE_ROOT)/img/copy_file_to_image.sh $(ACE_ROOT)/img/floppy.ima $(TARGET) .

generic:	always
	make -C generic 

clean:
	make -C generic clean
	rm	$(OBJS:.o=.d)
	rm	$(OBJS)
	rm	$(TARGET)

include $(OBJS:.o=.d)

always:

