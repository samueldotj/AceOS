#! /bin/sh
if test -z "$QEMU_BIOS_DIR"
then 
	echo "Set the QEMU_BIOS_DIR environment variable. This variable should point to QEMU bios and video bios directory"
	exit
fi

SCRIPT_PATH=`dirname $0`
qemu -L "$QEMU_BIOS_DIR" -M pc -m 32 -no-kqemu -hdc $SCRIPT_PATH/../build/c.img -boot n -net nic,model=pcnet,macaddr=52:54:00:12:34:58,vlan=0 -net tap,vlan=0,ifname=tap0