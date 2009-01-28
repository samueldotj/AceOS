#src/drivers/makefile

include $(ACE_ROOT)/make.conf
ALL_DIRS=	pci

all:	always
	@for dir in $(ALL_DIRS); do make -C $$dir all; done

pci:	always
	@make -C pci

clean:	always
	@for dir in $(ALL_DIRS); do make -C $$dir clean; done

always:	
