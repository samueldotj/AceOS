rm ./leak_info.txt 
./testheap /fifo /lifo /all_random /free_random /alloc_count 2000 $*
cat ./leak_info.txt
