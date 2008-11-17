#! /bin/sh
if test -z "$ACE_ROOT"
then 
	echo "Set the ACE_ROOT environment variable first"
	exit
fi

#create kernel boot module container
$ACE_ROOT/obj/mkmc -o $ACE_ROOT/obj/boot_modules.mod $ACE_ROOT/src/kernel/test.o

mkdir -p $ACE_ROOT/img/iso/boot/grub
cp $ACE_ROOT/img/boot/grub/stage2_eltorito $ACE_ROOT/img/iso/boot/grub
cp $ACE_ROOT/img/boot/grub/menu.lst $ACE_ROOT/img/iso/boot/grub
cp $ACE_ROOT/obj/kernel.sys $ACE_ROOT/img/iso/
cp $ACE_ROOT/obj/boot_modules.mod $ACE_ROOT/img/iso/

mkisofs -quiet -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table -o $ACE_ROOT/img/bootcd.iso $ACE_ROOT/img/iso
