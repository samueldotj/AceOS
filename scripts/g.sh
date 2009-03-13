#! /bin/sh

SCRIPT_PATH=`dirname $0`
gdb -q --command=$SCRIPT_PATH/gdb/.gdbinit --directory=$SCRIPT_PATH/../src/kernel --symbols=$SCRIPT_PATH/../build/default/src/kernel/kernel.sys