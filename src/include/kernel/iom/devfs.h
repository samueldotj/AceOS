/*! \file 	kernel/iom/devfs.h
    \brief 	devfs metadata structures
*/

#ifndef DEVFS_H
#define DEVFS_H

#include <ace.h>
#include <ds/avl_tree.h>
#include <ds/list.h>
#include <ds/lrulist.h>
#include <heap/slab_allocator.h>

/*! maximum length of a special file*/
#define DEVFS_FILE_NAME_MAX		50

/*! a special file's directory entry*/
typedef struct devfs_metadata
{
	char				name[DEVFS_FILE_NAME_MAX];		/*! name of the special file */
	DEVICE_OBJECT_PTR	device;							/*! device associated with the file*/
	
	AVL_TREE			tree;							/*! tree of files*/
}DEVFS_METADATA, * DEVFS_METADATA_PTR;

extern AVL_TREE_PTR	devfs_root;

#ifdef __cplusplus
    extern "C" {
#endif

ERROR_CODE CreateDeviceNode(const char * filename, DEVICE_OBJECT_PTR device);

#ifdef __cplusplus
	}
#endif

#endif

