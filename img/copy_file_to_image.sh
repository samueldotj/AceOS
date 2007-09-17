#! /bin/sh

free_loop=`losetup -f`
losetup $free_loop $1

mkdir $ACE_ROOT/temp
mount $free_loop $ACE_ROOT/temp

cp $2 $ACE_ROOT/temp/$3

umount $ACE_ROOT/temp
rm -rf $ACE_ROOT/temp

losetup -d $free_loop
