#ifndef _TEST_COMMON_H_
#define _TEST_COMMON_H_

#include <heap/slab_allocator.h>
#include <heap/heap.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/mman.h>

#define TEST_TYPE_FIFO			1
#define TEST_TYPE_LIFO			2
#define TEST_TYPE_FREE_RAND		4
#define TEST_TYPE_ALL_RAND		8

#define PAGE_SIZE	4096

extern int alloc_count, cache_size, min_slabs, free_slabs_threshold, max_slabs;

extern int verbose_level;
extern int test_type;

int parse_arguments(int argc, char * argv[]);
void fill_random_numbers(int * number_array, int capacity, int max_number);
void print_stats(CACHE_PTR cache_ptr);
int rand();
void srand(unsigned int seed);
void exit(int status);
int GetRandomNumber(int min, int max);

void free(void *);

void * virtual_alloc(int size);
int virtual_free(void * va, int size);
int virtual_protect(void * va, int size, int protection);

#endif
