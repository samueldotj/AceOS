#! /bin/sh
if test -z "$QEMU_BIOS_DIR"
then 
	echo "Set the QEMU_BIOS_DIR environment variable. This variable should point to QEMU bios and video bios directory"
	exit
fi

SCRIPT_PATH=`dirname $0`
#create_bootcd.sh
qemu -L "$QEMU_BIOS_DIR" -M pc -cdrom $SCRIPT_PATH/../build/bootcd.iso -boot d -m 32 -smp 1 -localtime -no-kqemu $*

