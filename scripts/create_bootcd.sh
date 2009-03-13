#! /bin/sh

BUILD_DIR=$1/src/
ACE_ROOT=$BUILD_DIR/../../../
ISO_DIR=$BUILD_DIR/../../iso
TOOLS_DIR=$ACE_ROOT/src/tools/build/default/
GRUB_BIN=$ACE_ROOT/boot/grub/

#directory clean up and create if needed
rm -rf $ISO_DIR
mkdir -p $ISO_DIR
mkdir -p $ISO_DIR/boot/grub

#create kernel boot module container
rm -f $ISO_DIR/boot_modules.mod
$TOOLS_DIR/mc/mkmc -o $ISO_DIR/boot_modules.mod $BUILD_DIR/app/hello.exe $BUILD_DIR/drivers/pci_bus.sys
gzip $ISO_DIR/boot_modules.mod

#copy grub
cp $GRUB_BIN/stage2_eltorito $ISO_DIR/boot/grub
cp $GRUB_BIN/menu.lst $ISO_DIR/boot/grub
cp $BUILD_DIR/kernel/kernel.sys $ISO_DIR

#create iso file
mkisofs -quiet -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table -o $ISO_DIR/../bootcd.iso $ISO_DIR
