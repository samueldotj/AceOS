#! /bin/sh
if test -z "$ACE_ROOT"
then 
	echo "Set the ACE_ROOT environment variable first"
	exit
fi

OBJ=$ACE_ROOT/obj/
IMG=$ACE_ROOT/img/
TOOLS_BIN=$ACE_ROOT/obj/tools/
USR_BIN=$ACE_ROOT/obj/usr/bin/

rm -f $ACE_ROOT/obj/boot_modules.mod.gz
#create kernel boot module container
$TOOLS_BIN/mkmc -v -o $OBJ/boot_modules.mod $USR_BIN/hello $OBJ/pci_bus.sys
gzip $OBJ/boot_modules.mod

rm -rf $IMG/iso/
mkdir -p $IMG/iso/boot/grub
cp $IMG/boot/grub/stage2_eltorito $IMG/iso/boot/grub
cp $IMG/boot/grub/menu.lst $IMG/iso/boot/grub
cp $OBJ/kernel.sys $IMG/iso
cp $OBJ/boot_modules.mod.gz $IMG/iso

mkisofs -quiet -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table -o $IMG/bootcd.iso $IMG/iso
