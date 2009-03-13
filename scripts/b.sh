#! /bin/sh
#used alias to run bochs

SCRIPT_PATH=`dirname $0`
if [ $OSTYPE = cygwin ] ; then
	SCRIPT_PATH=`cygpath -a -w $SCRIPT_PATH`
fi
#create_bootcd.sh
bochs -q -f $SCRIPT_PATH/bochsrc