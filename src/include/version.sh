#/usr/bin/sh
#this script will extracts the svn revision number and svn time stamp and puts it in the build.h
#expects the first argument to be the INCLUDE path
echo '#define ACE_BUILD "'`svn info | grep Revision` > tmp
echo ' ' >> tmp
svn info | grep -e "Last Changed Date" | cut -c 20- >> tmp
echo '"' >> tmp
#remove the line breaks
sed ':a;N;$!ba;s/\n//g' < tmp > tmp1
#add one new line to avoid gcc warning message
echo "" >> tmp1

#copy the file only if it is different
cmp tmp1 $1/build.h > tmp
if [ $? -ne 0 ]
then
	mv tmp1 $1/build.h
fi
rm -f tmp tmp1