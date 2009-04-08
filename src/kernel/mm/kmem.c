/*! \file 	kernel/mm/kmem.c
	\brief	Kernel memory allocation / free functions
*/
#include <ds/align.h>
#include <kernel/debug.h>
#include <kernel/mm/kmem.h>
#include <kernel/mm/pmem.h>
#include <kernel/pm/task.h>
#include <kernel/pm/thread.h>
#include <kernel/pm/pid.h>
#include <kernel/vfs/vfs.h>
#if ARCH == i386
#include <kernel/i386/pmem.h>
#endif

static void * kmem_page_alloc(int size);
static int kmem_page_free(void * va, int size);
static int kmem_page_protect(void * va, int size, int protection);
UINT32 kmem_reserved_mem_size=0;

/*! Initializes kernel memory allocator
 */
void InitKmem()
{
	if ( kmem_reserved_mem_size==0 )
		kmem_reserved_mem_size = (KMEM_RESERVED_MEM_PERCENTAGE * vm_data.total_memory_pages) /100;
		
	/*reserve some memory for heap*/
	kmem_reserved_mem_size = PAGE_ALIGN_UP(kmem_reserved_mem_size*PAGE_SIZE);
	kernel_reserve_range.kmem_va_end = (VADDR)((char*)kernel_reserve_range.kmem_va_start)+kmem_reserved_mem_size;
	kprintf("kmem: preallocating memory from 0x%p to 0x%p (%d KB)\n", kernel_reserve_range.kmem_va_start, kernel_reserve_range.kmem_va_end, kmem_reserved_mem_size/1024 );
	if ( MapVirtualAddressRange( &kernel_physical_map, kernel_reserve_range.kmem_va_start, kmem_reserved_mem_size, PROT_WRITE) != ERROR_SUCCESS )
		panic("InitKmem() - MapVirtualAddressRange() failed.");
	/*initialize heap*/
	if ( InitHeap(PAGE_SIZE, kmem_page_alloc, kmem_page_free, kmem_page_protect) )
		panic("InitKmem() - InitHeap() failed.");
	if ( AddMemoryToHeap((char *)kernel_reserve_range.kmem_va_start, (char*)kernel_reserve_range.kmem_va_end ) != 0 )
		panic("InitKmem() -  AddMemoryToHeap() failed.");
	
	/*intialize all kernel slab allocators*/
	if ( InitCache(&thread_cache, sizeof(THREAD_CONTAINER), THREAD_CACHE_FREE_SLABS_THRESHOLD, THREAD_CACHE_MIN_SLABS, THREAD_CACHE_MAX_SLABS, &ThreadCacheConstructor, &ThreadCacheDestructor) == -1 )
		panic("InitCache(thread_cache) failed");

	if ( InitCache(&task_cache, sizeof(TASK), TASK_CACHE_FREE_SLABS_THRESHOLD, TASK_CACHE_MIN_SLABS, TASK_CACHE_MAX_SLABS, &TaskCacheConstructor, &TaskCacheDestructor) == -1 )
		panic("InitCache(task_cache) failed");
	
	if ( InitCache(&pid_cache, sizeof(PID_INFO), 0, 0, 0, PidCacheConstructor, PidCacheDestructor) == -1 )
		panic("InitCache(pid_cache) failed");
		
	if ( InitCache(&virtual_map_cache, sizeof(VIRTUAL_MAP), TASK_CACHE_FREE_SLABS_THRESHOLD, TASK_CACHE_MIN_SLABS, TASK_CACHE_MAX_SLABS, &VirtualMapCacheConstructor, &VirtualMapCacheDestructor) == -1 )
		panic("InitCache(virtual_map_cache) failed");
	
	if ( InitCache(&vm_descriptor_cache, sizeof(VM_DESCRIPTOR), TASK_CACHE_FREE_SLABS_THRESHOLD*10, TASK_CACHE_MIN_SLABS*10, TASK_CACHE_MAX_SLABS*10, &VirtualMapCacheConstructor, &VirtualMapCacheDestructor) == -1 )
		panic("InitCache(vm_descriptor_cache) failed");
	//AddMemoryToCache(&vm_descriptor_cache, (char *)kernel_reserve_range.kmem_va_start, (char *)kernel_reserve_range.kmem_va_end);
	
	if ( InitCache(&physical_map_cache, sizeof(PHYSICAL_MAP), 0, 0, 0, PhysicalMapCacheConstructor, PhysicalMapCacheDestructor) == -1 )
		panic("InitCache(physical_map_cache) failed");
		
	if ( InitCache(&dir_entry_cache, sizeof(DIRECTORY_ENTRY), fs_param.dir_entry.free_slabs_threshold, fs_param.dir_entry.min_buffers, fs_param.dir_entry.max_buffers, DirEntryCacheConstructor, DirEntryCacheDestructor) == -1 )
		panic("InitCache(dir_entry_cache) failed");
		
	if ( InitCache(&vnode_cache, sizeof(VNODE), fs_param.dir_entry.free_slabs_threshold, fs_param.dir_entry.min_buffers, fs_param.dir_entry.max_buffers, VnodeCacheConstructor, VnodeCacheDestructor) == -1 )
		panic("InitCache(vnode_cache) failed");
	
}

/*! kmem Wrapper function for AllocateVirtualMemory
  * \param size - size in bytes(should be PAGE_SIZE aligned)
 */
static void * kmem_page_alloc(int size)
{
	VADDR va;
	ERROR_CODE ret;
#ifdef KMEM_DEBUG
	if ( size <= 0 || size % PAGE_SIZE )
	{
		kprintf("kmem_page_alloc(%d)\n", size);
		panic("kmem_page_alloc() - size is incorrect\n");
	}
#endif
	ret = AllocateVirtualMemory(&kernel_map, &va, 0, size, 0, 0, NULL);
	if ( ret == ERROR_SUCCESS )
		return (void *)va;
	else
		return NULL;
}

/*! Kmem wrapper function for FreeVirtualMemory
 * \param va	-	address which should be freed
 * \param size	-	size of the range
 */
static int kmem_page_free(void * va, int size)
{
#ifdef KMEM_DEBUG
	if ( va <= 0 || ((unsigned long)va) % PAGE_SIZE)
		panic("kmem_page_free() - va is incorrect\n");
	/*size should be greater than 0 and should be a multiple of page size*/
	if ( size <= 0 || size % PAGE_SIZE )
		panic("kmem_page_free() - size is incorrect\n");
#endif
	return FreeVirtualMemory(&kernel_map, (VADDR)va, size, NULL);
}

/*!	Updates protection right for a given kmem range
 * \todo implement protection
 */
static int kmem_page_protect(void * va, int size, int protection)
{
#ifdef KMEM_DEBUG
	if ( va <= 0 || ((unsigned long)va) % PAGE_SIZE)
		panic("kmem_page_protect() - va is incorrect\n");
	/*size should be greater than 0 and should be a multiple of page size*/
	if ( size <= 0 || size % PAGE_SIZE )
		panic("kmem_page_protect() - size is incorrect\n");
#endif
	return 1;
}

/*!
 * \brief Allocates memory from kernel memory allocator
 * \param size - required size in bytes
 * \param flag - KMEM_NO_FAIL 
 */
void * kmalloc(int size, UINT32 flag)
{
	void * ret = AllocateFromHeap(size);
	if ( ret==NULL && (flag & KMEM_NO_FAIL) )
	{
		kprintf("AllocateFromHeap(%d) failed.\n", size);
		panic("kmalloc failed");
	}
	return ret;
}

/*!
 * \brief Frees memory back to kernel memory allocator
 * \param buffer - starting address of memory to free
 * \return 0 on success
 */
int kfree(void * buffer)
{
	return FreeToHeap(buffer);
}
