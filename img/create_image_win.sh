#! /bin/sh
if test -z "$ACE_ROOT"
then 
	echo "Set the ACE_ROOT environment variable first"
	exit
else
	echo "ACE_ROOT=$ACE_ROOT"
fi

echo "yes\n\r ok^m" > yes

#filedisk and bximage requires absolute path in windows format
WINDOWS_PATH_RAW_IMA=`cygpath -a -w $ACE_ROOT/img/raw.ima`
WINDOWS_PATH_FAT_IMA=`cygpath -a -w $ACE_ROOT/img/FAT.ima`

RAW_DRIVE="p:"
FAT_DRIVE="q:"

WINDOWS_PATH_HARDDISK1_IMG=`cygpath -a -w $ACE_ROOT/img/c.img`
WINDOWS_PATH_HARDDISK2_IMG=`cygpath -a -w $ACE_ROOT/img/d.img`

HARDISK1_DRIVE="p:"
HARDISK2_DRIVE="q:"

#raw.ima - temporary grub disk. FAT.ima - Grub with FAT FS 
echo "creating $WINDOWS_PATH_RAW_IMA. press any key to continue"
bximage -fd -q -size=1.44 $WINDOWS_PATH_RAW_IMA	< yes >> out
echo "creating $WINDOWS_PATH_FAT_IMA. press any key to continue"
bximage -fd -q -size=1.44 $WINDOWS_PATH_FAT_IMA < yes >> out

#mount the images using filedisk 
echo "mounting $WINDOWS_PATH_RAW_IMA at $RAW_DRIVE"
filedisk /mount 0 $WINDOWS_PATH_RAW_IMA 1.44M $RAW_DRIVE
echo "mounting $WINDOWS_PATH_FAT_IMA at $FAT_DRIVE"
filedisk /mount 1 $WINDOWS_PATH_FAT_IMA 1.44M $FAT_DRIVE 

#create temp grub disk in raw format 
command.com /C copy /b  boot\\grub\\stage1+boot\\grub\\stage2 grub.raw	>> out
echo "copying grub.raw $RAW_DRIVE"
ntrawrite -n -f grub.raw -d $RAW_DRIVE < yes >> out

#prepare the second disk with GRUB files and directory structure 
echo "formatting $FAT_DRIVE"
command.com /C format /fs:FAT /v:fdd $FAT_DRIVE < yes >> out

mkdir $FAT_DRIVE/grub
cp $ACE_ROOT/img/boot/grub/stage1 $FAT_DRIVE/grub
cp $ACE_ROOT/img/boot/grub/stage2 $FAT_DRIVE/grub
cp $ACE_ROOT/img/boot/grub/menu.lst $FAT_DRIVE/grub

echo "waiting for floppy operation to complete"
sleep 5

#all done - unmount the disks 
echo "unmounting floppy disks"
filedisk /umount $RAW_DRIVE
filedisk /umount $FAT_DRIVE

#rename the FAT to floppy
echo "renaming the file"
mv $ACE_ROOT/img/FAT.ima $ACE_ROOT/img/floppy.ima

echo "removing unwanted files"
#rm $ACE_ROOT/img/raw.ima
rm $ACE_ROOT/img/grub.raw

#creating hardisk
echo "creating harddisk1. press any key to continue"
bximage -q -hd -size=20 -mode=flat $WINDOWS_PATH_HARDDISK1_IMG	< yes >> out
echo "creating harddisk2. press any key to continue"
bximage -q -hd -size=20 -mode=flat $WINDOWS_PATH_HARDDISK2_IMG  < yes >> out

echo "mounting $WINDOWS_PATH_RAW_IMA at $RAW_DRIVE"
filedisk /mount 0 $WINDOWS_PATH_HARDDISK1_IMG $HARDISK1_DRIVE
echo "mounting $WINDOWS_PATH_FAT_IMA at $FAT_DRIVE"
filedisk /mount 1 $WINDOWS_PATH_HARDDISK2_IMG $HARDISK2_DRIVE 

#create filesystem in the harddisk  (ace supports only FAT)
echo "formating harddisk1"
command.com /C format /fs:FAT /v:hdd1 $HARDISK1_DRIVE	< yes >> out
echo "formating harddisk2"
command.com /C format /fs:FAT /v:hdd2 $HARDISK2_DRIVE 	< yes >> out

echo "unmounting harddisk"
filedisk /umount $HARDISK1_DRIVE
filedisk /umount $HARDISK2_DRIVE

rm -f yes