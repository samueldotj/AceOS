#	File			:	src makefile
#   Author			:	Samuel Jacob
#  	Version			:	3.0
#  	Created 		:	
#	Last modified	:
#	Brief			:	commands supported are: all clean 

include $(ACE_ROOT)/make.conf
ALL_DIRS=	

all:	always
	@for dir in $(ALL_DIRS); do make -C $$dir; done

clean:	always
	@for dir in $(ALL_DIRS); do make -C $$dir clean; done

always:	
