#ds/sync/makefile

include $(ACE_ROOT)/make.conf

#clean dirs 
CLEAN_DIRS=	$(ARCH)

#target path
TARGET= $(OBJ)/libsync.a

#individual objects
GENERIC_OBJ = $(patsubst %.c,%.o,$(wildcard *.c))
ARCH_OBJ = $(patsubst %.c,%.o,$(wildcard $(ARCH)/*.c)) $(patsubst %.asm,%.o,$(wildcard $(ARCH)/*.asm)) 

#all objects for this library
OBJS = $(ARCH_OBJ) $(GENERIC_OBJ)

#phony - all - make target
all:    $(TARGET)

#phony - clean - clean all object files
clean:
	@rm	-f *.o 
	@for dir in $(CLEAN_DIRS); do make -C $$dir clean; done
	@rm	-f $(TARGET)

#phony - clean - clean including .d files
all_clean: clean
	@rm	-f *.d

#how to make target
$(TARGET):  $(OBJS)
	@$(AR) ru $(TARGET) $(OBJS)

#create .d files
-include $(OBJS:.o=.d)
