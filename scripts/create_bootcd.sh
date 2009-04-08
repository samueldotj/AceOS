#! /bin/sh

BUILD_DIR=$1/src/
ACE_ROOT=$BUILD_DIR/../../../
ISO_DIR=$BUILD_DIR/../../iso
GRUB_BIN=$ACE_ROOT/boot/grub/

#directory clean up and create if needed
rm -rf $ISO_DIR
mkdir -p $ISO_DIR
mkdir -p $ISO_DIR/boot/grub

#create boot fs
rm -f $ISO_DIR/boot_modules.mod.gz
rm -rf $BUILD_DIR/bootfs
mkdir -p $BUILD_DIR/bootfs/app
mkdir -p $BUILD_DIR/bootfs/drivers
cp $ACE_ROOT/src/kernel/driver_id.txt $BUILD_DIR/bootfs
cp $BUILD_DIR/app/hello.exe $BUILD_DIR/bootfs/app
cp $BUILD_DIR/drivers/pci_bus.sys $BUILD_DIR/bootfs/drivers
cd $BUILD_DIR/bootfs
tar --gzip -cf $ISO_DIR/boot_modules.mod.gz .

#copy grub
cp $GRUB_BIN/stage2_eltorito $ISO_DIR/boot/grub
cp $GRUB_BIN/menu.lst $ISO_DIR/boot/grub
cp $BUILD_DIR/kernel/kernel.sys $ISO_DIR

#create iso file
mkisofs -quiet -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table -o $ISO_DIR/../bootcd.iso $ISO_DIR
