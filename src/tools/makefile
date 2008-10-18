#	File			:	src/tools/makefile
#	Brief			:	commands supported are: ds string all clean 

include $(ACE_ROOT)/make.conf
ALL_DIRS=	mc

all:	always
	@for dir in $(ALL_DIRS); do make -C $$dir all; done

clean:	always
	@for dir in $(ALL_DIRS); do make -C $$dir clean; done

always:	
