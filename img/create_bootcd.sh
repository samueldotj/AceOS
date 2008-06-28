#! /bin/sh
if test -z "$ACE_ROOT"
then 
	echo "Set the ACE_ROOT environment variable first"
	exit
else
	echo "ACE_ROOT=$ACE_ROOT"
fi

mkdir -p $ACE_ROOT/img/iso/boot/grub
cp $ACE_ROOT/img/boot/grub/stage2_eltorito $ACE_ROOT/img/iso/boot/grub
cp $ACE_ROOT/img/boot/grub/menu.lst $ACE_ROOT/img/iso/boot/grub
cp $ACE_ROOT/obj/kernel.sys $ACE_ROOT/img/iso/

mkisofs -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table -o bootcd.iso iso
