#src/lib/makefile

include $(ACE_ROOT)/make.conf

ALL_DIRS=	string ds sync heap

all:	always
	@for dir in $(ALL_DIRS); do make -C $$dir all; done

string:	always
	@make -C string

ds:	always
	@make -C ds

heap:	always
	@make -C heap

sync:	always
	@make -C sync

clean:	always
	@for dir in $(ALL_DIRS); do make -C $$dir clean; done

all_clean:	clean
	@for dir in $(ALL_DIRS); do make -C $$dir all_clean; done

always:	
