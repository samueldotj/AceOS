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
testtree:	testtree.c
	$(CC) -I $(INCLUDE) -L $(OBJ) -o testtree testtree.c -lds

testlist:	testlist.c
	$(CC) -I $(INCLUDE) -L $(OBJ) -o testlist testlist.c -lds
	
testavl:	testavl.c
	$(CC) -I $(INCLUDE) -L $(OBJ) -o testavl testavl.c -lds
	
#create .d files
-include $(OBJS:.o=.d)
