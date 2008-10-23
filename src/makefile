#	File			:	src/makefile
#	Brief			:	commands supported are: kernel lib all clean 

include $(ACE_ROOT)/make.conf
ALL_DIRS=	lib kernel tools

all:	always
	@for dir in $(ALL_DIRS); do make -C $$dir all; done

kernel:	always
	@make -C kernel

lib:	always
	@make -C lib
	
tools:	always
	@make -C tools

clean:	always
	@for dir in $(ALL_DIRS); do make -C $$dir clean; done

always:	
