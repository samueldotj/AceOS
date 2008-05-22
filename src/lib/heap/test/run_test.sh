rm ./leak_info.txt
#./testslab /fifo /lifo /all_random /free_random /alloc_count 30 /cache_size 40 /min_slabs 10 /max_slabs 20 $*
#./testslab /fifo /lifo /all_random /free_random /alloc_count 100 /cache_size 400 /min_slabs 100 /max_slabs 100 $*
./testslab /fifo /lifo /all_random /free_random /alloc_count 100 /cache_size 40 /min_slabs 10 /max_slabs 100 $*
cat ./leak_info.txt
