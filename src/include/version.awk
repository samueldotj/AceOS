/#define ACE_BUILD/ { print "#define ACE_BUILD " "\" " $4+1 " (" strftime("%d-%b-%Y %H:%M") ")\""; }
$0 !~/#define ACE_BUILD/ { print ; }
