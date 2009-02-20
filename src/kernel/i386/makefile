#kernel/i386/makefile

include $(ACE_ROOT)/make.conf

TARGET= $(OBJ)/arch.a

ARCH_OBJ= $(patsubst %.c,%.o,$(wildcard *.c)) $(patsubst %.asm,%.o,$(wildcard *.asm)) 
MM_OBJ= $(patsubst %.c,%.o,$(wildcard mm/*.c)) $(patsubst %.asm,%.o,$(wildcard mm/*.asm)) 
PM_OBJ= $(patsubst %.c,%.o,$(wildcard pm/*.c)) $(patsubst %.asm,%.o,$(wildcard pm/*.asm))
DEBUG_OBJ= $(patsubst %.c,%.o,$(wildcard debug/*.c)) $(patsubst %.asm,%.o,$(wildcard debug/*.asm))
PIC_OBJ= $(patsubst %.c,%.o,$(wildcard pic/*.c)) $(patsubst %.asm,%.o,$(wildcard pic/*.asm))

OBJS= 	$(ARCH_OBJ) $(MM_OBJ) $(PM_OBJ) $(DEBUG_OBJ) $(PIC_OBJ) 

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
