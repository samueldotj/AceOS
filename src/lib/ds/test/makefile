#	File			:	lib/ds/test makefile
#   Author			:	Samuel Jacob
#  	Version			:	3.0
#  	Created 		:	21-Feb-2008 13:43
#	Last modified	:
#	Brief			:	commands supported are: clean testtree testlist

include $(ACE_ROOT)/make.conf

#since we might use different cross compiler for Ace, for testing use the host's default compiler.
CC = /usr/bin/gcc

#phony - all - make target
all:    testtree testlist testavl

#phony - clean - clean all object files
clean:
	@rm -f *.d *.o
	@rm -f *.exe

#how to make target
testcommon.o:	testcommon.c
	$(CC) -I $(INCLUDE) -c -o testcommon.o testcommon.c
	
testtree:	testcommon.o testtree.c $(OBJ)/libds.a
	$(CC) -I $(INCLUDE) -L $(OBJ) -o testtree testtree.c testcommon.o -lds

testlist:	testlist.c $(OBJ)/libds.a
	$(CC) -I $(INCLUDE) -L $(OBJ) -o testlist testlist.c -lds
	
testavl:	testcommon.o testavl.c $(OBJ)/libds.a
	$(CC) -I $(INCLUDE) -L $(OBJ) -o testavl testavl.c testcommon.o -lds
	
#create .d files
-include $(OBJS:.o=.d)
