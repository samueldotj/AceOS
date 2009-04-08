/*!
  \file		kernel/vfs/vfs_types.h
  \brief	all the typedefs for virtual file system
*/
#ifndef _VFS_TYPES_H
#define _VFS_TYPES_H

typedef struct file_system FILE_SYSTEM, * FILE_SYSTEM_PTR;
typedef struct mounted_file_system MOUNTED_FILE_SYSTEM, * MOUNTED_FILE_SYSTEM_PTR;
typedef struct vnode VNODE, * VNODE_PTR;
typedef struct directory_entry DIRECTORY_ENTRY, * DIRECTORY_ENTRY_PTR;
typedef struct open_file_info OPEN_FILE_INFO, * OPEN_FILE_INFO_PTR;
typedef struct process_file_info PROCESS_FILE_INFO, * PROCESS_FILE_INFO_PTR;
typedef struct file_stat_param FILE_STAT_PARAM, * FILE_STAT_PARAM_PTR;
typedef struct fs_control FS_CONTROL, * FS_CONTROL_PTR;
typedef struct fs_param FS_PARAM, * FS_PARAM_PTR;
typedef struct directory_entry_param DIRECTORY_ENTRY_PARAM, * DIRECTORY_ENTRY_PARAM_PTR;

#endif
