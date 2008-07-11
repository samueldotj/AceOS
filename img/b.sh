#! /bin/sh
#used alias to run bochs

if [ $OSTYPE = cygwin ] ; then
	export ACE_ROOT=`cygpath -a -w $ACE_ROOT`	
fi
create_bootcd.sh
bochs -q -f $ACE_ROOT/img/bochsrc