#/usr/bin/sh
#this script will increment the build number and also put the build time stamp in the version.h
#expects the first argument to be the INCLUDE path
cp $1/version.h version.old
gawk -f $1/version.awk version.old > $1/version.h
rm version.old