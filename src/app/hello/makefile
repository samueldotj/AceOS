#app/hello/makefile

include $(ACE_ROOT)/make_app.conf

TARGET=$(USR_BIN)/hello

#how to make target
$(TARGET):	hello.c
	$(CC) $(CFLAGS) -o $(TARGET) hello.c -lc -lm

#phony - clean - clean all object files
clean:
	@rm -f *.d *.o
	@rm -f $(TARGET)

#create .d files
-include $(OBJS:.o=.d)
