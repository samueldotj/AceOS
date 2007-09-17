#! /bin/sh
if test -z "$ACE_ROOT"
then 
	echo "Set the ACE_ROOT variable first![$ACE_ROOT]"
	exit
else
	echo "ACE_ROOT=$ACE_ROOT"
fi

#Commands: 
#raw.ima - temporary grub disk. FAT.ima - Grub with FAT FS 

bximage -fd -q -size=1.44 $ACE_ROOT/img/raw.ima 
bximage -fd -q -size=1.44 $ACE_ROOT/img/FAT.ima 

#setting loopback device.
losetup /dev/loop0 $ACE_ROOT/img/raw.ima
losetup /dev/loop1 $ACE_ROOT/img/FAT.ima

#mount the images
mkdosfs /dev/loop1
mkdir -p $ACE_ROOT/img/mnt/FAT
mount -t msdos /dev/loop1  $ACE_ROOT/img/mnt/FAT

#create temp grub disk in raw format 
dd if=$ACE_ROOT/img/boot/grub/stage1 of=/dev/loop0 bs=512 count=1
dd if=$ACE_ROOT/img/boot/grub/stage2 of=/dev/loop0 bs=512 seek=1

#prepare the second disk with GRUB files and directory structure 
mkdir $ACE_ROOT/img/mnt/FAT/grub 
cp -f $ACE_ROOT/img/boot/grub/stage1 $ACE_ROOT/img/mnt/FAT/grub/.
cp -f $ACE_ROOT/img/boot/grub/stage2 $ACE_ROOT/img/mnt/FAT/grub/.
cp -f $ACE_ROOT/img/boot/grub/menu.lst $ACE_ROOT/img/mnt/FAT/grub/.

#all done - unmount the disks 
umount $ACE_ROOT/img/mnt/FAT 

losetup -d /dev/loop0
losetup -d /dev/loop1

mv $ACE_ROOT/img/FAT.ima $ACE_ROOT/img/floppy.ima


#creating hardisk
bximage -q -hd -size=20 -mode=flat c.img
bximage -q -hd -size=20 -mode=flat d.img

mkdosfs c.img
mkdosfs d.img
