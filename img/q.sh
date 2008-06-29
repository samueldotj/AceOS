#! /bin/sh
if test -z "$QEMU_BIOS_DIR"
then 
	echo "Set the QEMU_BIOS_DIR environment variable. This variable should point to QEMU bios and video bios directory"
	exit
fi

if [ $OSTYPE = cygwin ] ; then
	export ACE_ROOT=`cygpath -a -w $ACE_ROOT`
fi

qemu -L "$QEMU_BIOS_DIR" -M pc -hda $ACE_ROOT/img/c.img -hdb $ACE_ROOT/img/d.img -cdrom $ACE_ROOT/img/bootcd.iso -boot d -m 32 -smp 4 -localtime -no-kqemu $*