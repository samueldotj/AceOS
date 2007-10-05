#! /bin/sh
ACE_ROOT=$PWD
export ACE_ROOT
echo $ACE_ROOT
export PATH=$ACE_ROOT/img:$PATH:
export QEMU_BIOS_DIR="c:/Program Files/Qemu/bios"
bash --login -i
