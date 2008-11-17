#src/makefile

include $(ACE_ROOT)/make.conf
ALL_DIRS=	lib kernel tools

all:	always
	@for dir in $(ALL_DIRS); do make -C $$dir all; done

lib:	always
	@make -C lib

kernel:	always
	@make -C kernel

tools:	always
	@make -C tools

clean:	always
	@for dir in $(ALL_DIRS); do make -C $$dir clean; done

always:	
