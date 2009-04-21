#! /bin/sh

QEMU=`which qemu`
SCRIPT_PATH=`dirname $0`

if test -z "$QEMU_BIOS_DIR"
then 
	QEMU_BIOS_DIR=`dirname $QEMU`
	if [ $OSTYPE = cygwin ] ; then
		QEMU_BIOS_DIR=`cygpath -a -w $QEMU_BIOS_DIR`
	fi
fi

qemu -L "$QEMU_BIOS_DIR" -M pc -cdrom $SCRIPT_PATH/../build/bootcd.iso -boot d -m 32 -smp 1 -localtime -no-kqemu $*
