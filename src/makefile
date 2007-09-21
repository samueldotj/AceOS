#	File			:	src makefile
#   Author			:	DilipSimha N M and Samuel Jacob
#  	Version			:	3.0
#  	Created 		:	20-Sep-2007 11:55 AM
#	Last modified	:
#	Brief			:	commands supported are: kernel all clean 

include $(ACE_ROOT)/make.conf
ALL_DIRS=	kernel

kernel:	always
	make -C kernel

all:	always
	for dir in $(ALL_DIRS); do make -C $$dir all; done

clean:	always
	for dir in $(ALL_DIRS); do make -C $$dir clean; done

always:	
