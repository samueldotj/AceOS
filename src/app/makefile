#src/app/makefile

include $(ACE_ROOT)/make_app.conf

ALL_DIRS=	hello

all:	always
	@for dir in $(ALL_DIRS); do make -C $$dir; done

hello:	always
	@make -C hello

clean:	always
	@for dir in $(ALL_DIRS); do make -C $$dir clean; done

always:	
	@mkdir -p $(USR_BIN)