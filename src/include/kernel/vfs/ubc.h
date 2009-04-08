/*! \file 	kernel/vfs/dir_entry.h
    \brief 	directory entry and cache realtead functions
*/

#ifndef UBC_H
#define UBC_H

VIRTUAL_PAGE_PTR GetVnodePage(VNODE_PTR vnode, VADDR offset);
ERROR_CODE FillUbcPage(VNODE_PTR vnode, VADDR offset, VIRTUAL_PAGE_PTR vp);
ERROR_CODE ReleaseVnodePages(VNODE_PTR vnode);

#endif
