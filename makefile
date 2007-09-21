#	File			:	Global makefile
#   Author			:	DilipSimha N M and Samuel Jacob
#  	Version			:	3.0
#  	Created 		:	20-Sep-2007 11:55 AM
#	Last modified	:
#	Brief			:	commands supported are: doc kernel src all clean all_clean	

include make.conf
ALL_DIRS=	src doc 

doc:	always
	make -C doc

kernel:	always
	make -C src/kernel

src:	always
	make -C src 

all:	always
	for dir in $(ALL_DIRS); do make -C $$dir all; done

clean:	always
	make -C src clean

all_clean:	always
	for dir in $(ALL_DIRS); do make -C $$dir clean; done

always:	
