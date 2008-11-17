#makefile
include make.conf

ALL_DIRS=	src doc 

src:	always
	make -C src 

doc:	always
	make -C doc

kernel:	always
	make -C src/kernel

all:	always
	for dir in $(ALL_DIRS); do make -C $$dir all; done

clean:	always
	make -C src clean

all_clean:	always
	for dir in $(ALL_DIRS); do make -C $$dir clean; done

always:	
