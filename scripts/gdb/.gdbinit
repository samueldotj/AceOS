#gdb command file

#debug remote debugging :)
#set remotelogfile gdb.log
#set debug remote 1

#serial port specific options
#set remoteaddresssize 32
#set remotebaud 115200

set print pretty on

#user defined commands
source gdb/general.gdb
source gdb/stack.gdb
source gdb/vm.gdb

#start debugging
#target remote /dev/com7
target remote localhost:1234