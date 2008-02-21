#	File			:	src makefile
#   Author			:	DilipSimha N M and Samuel Jacob
#  	Version			:	3.0
#  	Created 		:	21-Feb-2008 15:22PM
#	Last modified	:
#	Brief			:	commands supported are: ds string all clean 

include $(ACE_ROOT)/make.conf
ALL_DIRS=	string ds

all:	always
	@for dir in $(ALL_DIRS); do make -C $$dir all; done

string:	always
	@make -C string

ds:	always
	@make -C ds

clean:	always
	@for dir in $(ALL_DIRS); do make -C $$dir clean; done

always:	
