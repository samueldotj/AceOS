#! /bin/sh
if test -z "$QEMU_BIOS_DIR"
then 
	echo "Set the QEMU_BIOS_DIR environment variable. This variable should point to QEMU bios and video bios directory"
	exit
fi

if [ $OSTYPE = cygwin ] ; then
	qemu -L "$QEMU_BIOS_DIR" -M pc -fda `cygpath -a -w $ACE_ROOT/img/floppy.ima` -hda `cygpath -a -w $ACE_ROOT/img/c.img` -hdb `cygpath -a -w $ACE_ROOT/img/d.img` -boot a -m 32 -smp 2 -localtime -no-kqemu
else
	qemu -L "$QEMU_BIOS_DIR" -M pc -fda $ACE_ROOT/img/floppy.ima -hda $ACE_ROOT/img/c.img -hdb $ACE_ROOT/img/d.img -boot a -m 32 -smp 2 -localtime -no-kqemu -serial COM6
fi
