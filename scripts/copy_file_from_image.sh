#! /bin/sh
USAGE="Usage: `basename $0` <image> <image_file_to_copy> <local_path>"
if [ $# -lt 3 ] ; then
	echo "$USAGE"
	exit 1
fi

if [ ! -n "$OSTYPE" ]
then
	OSTYPE="LINUX"
fi

if [ $OSTYPE = "cygwin" ] ; then
	WINDOWS_PATH=`cygpath -a -w $1`
	filedisk /mount 0 $WINDOWS_PATH m:
	cp -u m:/$2 $3 
	filedisk /umount m:
else
	free_loop=`losetup -f`
	losetup $free_loop $1

	mkdir $ACE_ROOT/temp
	mount $free_loop $ACE_ROOT/temp

	cp -u $ACE_ROOT/temp/$2 $3

	umount $ACE_ROOT/temp
	rm -rf $ACE_ROOT/temp

	losetup -d $free_loop
fi

