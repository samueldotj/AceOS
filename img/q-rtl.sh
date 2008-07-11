#! /bin/sh
if test -z "$QEMU_BIOS_DIR"
then 
	echo "Set the QEMU_BIOS_DIR environment variable. This variable should point to QEMU bios and video bios directory"
	exit
fi

if [ $OSTYPE = cygwin ] ; then
	export ACE_ROOT=`cygpath -a -w $ACE_ROOT`
fi

qemu -L "$QEMU_BIOS_DIR" -M pc -m 32 -no-kqemu -hdc c.img -boot n -net nic,model=rtl8139,macaddr=52:54:00:12:34:57,vlan=0 -net tap,vlan=0,ifname=tap0