/*!
	\file	include/kernel/mm/virtual_page.h	
	\brief	virtual page
*/

#ifndef __VIRTUAL_PAGE_H
#define __VIRTUAL_PAGE_H

#include <ace.h>
#include <ds/list.h>
#include <sync/spinlock.h>
#include <kernel/mm/vm_types.h>

/*! Virtual Page to Physical address*/
#define VP_TO_PHYS(vp)		((vp)->physical_address)
/*! Physical Address to Virtual Page*/
#define PHYS_TO_VP(phys)	(PhysicalToVirtualPage(phys))

/*! Virtual Page representing a physical frame*/
struct virtual_page
{
	SPIN_LOCK	lock;
	
	BYTE
				free:1,				/*! if set page is free list*/
				active:1,			/*! if set page is in active LRU list*/
				ubc:1,				/*! if set page is allocated for file cache*/
				bad:1,				/*! if set page is bad*/
				busy:1,				/*! if set page is busy due to IO*/
				error:1,			/*! if set a page error occurred during last IO*/
				reserved;
	union
	{
		/*the following structure is used when the page in FREE state*/
		struct
		{
			VIRTUAL_PAGE_PTR	free_first_page;/*! pointer to starting page of this free physical range*/
			AVL_TREE_D			free_tree;		/*! this is starting of a free physical range*/
			UINT32				free_size;  	/*! size of the free range in page units*/
		};
		/*the following structure is used when the page is in USE state*/
		struct
		{
			LIST				lru_list;			/*! LRU List - active/inactive link*/
 
			VA_MAP_PTR			va_map_list;		/*! list of VAs associated with this page*/
		};
		/*the following structure is used when the page is controlled by ubc*/
		struct
		{
			AVL_TREE			tree;				/*! tree of all pages in a vnode - for faster search*/
			VNODE_PTR			vnode;				/*! back pointer to vnode - neccessary?*/
			VADDR				offset;				/*! file offset this page maps - key to the above tree*/
			BYTE				loaded:1,			/*! set to 1 if the page is loaded from file*/
								modified:1;			/*! set to 1 if the page is modified after load*/
		}ubc_info;
	};

	UINT16		copy_on_write;		/*! No of processes sharing this page*/
	UINT16 		wire_count;			/*! Total wire count*/
	
	UINT32 		physical_address;	/*Physical address this page managing*/
}__attribute__ ((packed));

/*! maps a physical frame with one or more virtual page - va
*/
struct va_map
{
	UINT32				va;					/*! virtual address for the mapping*/
	PHYSICAL_MAP_PTR	physical_map;		/*! pointer to the physical map for va*/
	
	LIST				list;				/*! list of all va_map for the virtual page*/
}__attribute__ ((packed));;

enum VIRTUAL_PAGE_RANGE_TYPE
{
	VIRTUAL_PAGE_RANGE_TYPE_NORMAL,
	VIRTUAL_PAGE_RANGE_TYPE_BELOW_1MB,
	VIRTUAL_PAGE_RANGE_TYPE_BELOW_16MB,
};

UINT32 InitVirtualPageArray(VIRTUAL_PAGE_PTR vpa, UINT32 page_count, UINT32 free_count, UINT32 start_physical_address);

VIRTUAL_PAGE_PTR AllocateVirtualPages(int pages, enum VIRTUAL_PAGE_RANGE_TYPE vp_range_type);
UINT32 FreeVirtualPages(VIRTUAL_PAGE_PTR vp, int pages);

VIRTUAL_PAGE_PTR PhysicalToVirtualPage(UINT32 physical_address);

UINT32 LockVirtualPages(VIRTUAL_PAGE_PTR first_vp, int pages);
UINT32 ReserveVirtualPages(VIRTUAL_PAGE_PTR first_vp, int pages);

extern UINT32 limit_physical_memory;
#endif
