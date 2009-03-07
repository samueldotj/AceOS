#src/makefile

include $(ACE_ROOT)/make.conf
ALL_DIRS=	lib drivers tools app kernel 

all:	always
	#do make all
	@for dir in $(ALL_DIRS); do make -C $$dir all; done
	#create boot cd 
	$(ACE_ROOT)/img/create_bootcd.sh

lib:	always
	@make -C lib

kernel:	always
	@make -C kernel
	
tools:	always
	@make -C tools

clean:	always
	@for dir in $(ALL_DIRS); do make -C $$dir clean; done

all_clean:	clean
	@for dir in $(ALL_DIRS); do make -C $$dir all_clean; done

always:	
