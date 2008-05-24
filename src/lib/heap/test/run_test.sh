rm ./leak_info.txt
./testslab /fifo /lifo /all_random /free_random /alloc_count 2000 /cache_size 100 /min_slabs 10 /max_slabs 20 $*
./testslab /fifo /lifo /all_random /free_random /alloc_count 1000 /cache_size 400 /min_slabs 100 /max_slabs 100 $*
./testslab /fifo /lifo /all_random /free_random /alloc_count 300 /cache_size 40 /min_slabs 10 /max_slabs 100 $*
./testslab /fifo /lifo /all_random /free_random /alloc_count 800 /cache_size 30 /min_slabs 10 /max_slabs 100 $*
./testslab /fifo /lifo /all_random /free_random /alloc_count 150 /cache_size 240 /min_slabs 10 /max_slabs 100 $*
./testslab /fifo /lifo /all_random /free_random /alloc_count 2400 /cache_size 24 /min_slabs 10 /max_slabs 100 $*
cat ./leak_info.txt
