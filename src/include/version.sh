#/usr/bin/sh
#this script will extracts the svn revision number and svn time stamp and puts it in the build.h
#expects the first argument to be the INCLUDE path
echo '#define ACE_BUILD "'`svn info | grep Revision` > tmp
echo ' ' >> tmp
svn info | grep -e "Last Changed Date" | cut -c 20- >> tmp
echo '"' >> tmp
#remove the line breaks
sed ':a;N;$!ba;s/\n//g' < tmp > $1/build.h
rm tmp
#add one new line to avoid gcc warning message
echo "" >> $1/build.h