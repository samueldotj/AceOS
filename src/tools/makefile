#rc/tools/makefile

include $(ACE_ROOT)/make.conf
ALL_DIRS=	mc

all:	always
	@for dir in $(ALL_DIRS); do make -C $$dir all; done

clean:	always
	@for dir in $(ALL_DIRS); do make -C $$dir clean; done

all_clean:	always
	@for dir in $(ALL_DIRS); do make -C $$dir all_clean; done

always:	
	@mkdir -p $(TOOLS_BIN)
