#! /bin/sh
if test -z "$QEMU_BIOS_DIR"
then 
	echo "Set the QEMU_BIOS_DIR environment variable. This variable should point to QEMU bios and video bios directory"
	exit
fi

PWD='pwd'
cd $ACE_ROOT/img
#create_bootcd.sh
qemu -L "$QEMU_BIOS_DIR" -M pc -cdrom bootcd.iso -boot d -m 32 -smp 1 -localtime -no-kqemu $*
cd $PWD
