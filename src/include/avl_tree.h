/*!
  \file		avl_tree.h
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created:
  			Last modified: Wed Feb 27, 2008  10:57AM
  \brief	
*/
#ifndef _AVL_TREE_H_
#define _AVL_TREE_H_

#include <binary_tree.h>


/*structures go here*/
struct avl_tree {
	BINARY_TREE	bintree;
    int			height;
};
typedef struct avl_tree  AVL_TREE, *AVL_TREE_PTR;

AVL_TREE_PTR SearchAvlTree(AVL_TREE_PTR start, AVL_TREE_PTR search_node);
int InsertNodeIntoAvlTree(AVL_TREE_PTR *root, AVL_TREE_PTR new_node);
int RemoveNodeFromAvlTree(AVL_TREE_PTR *root, AVL_TREE_PTR node);


#define AVL_TREE_LEFT_NODE(node)	((AVL_TREE_PTR)TREE_LEFT_NODE((BINARY_TREE_PTR)(node)))
#define AVL_TREE_RIGHT_NODE(node)	((AVL_TREE_PTR)TREE_RIGHT_NODE((BINARY_TREE_PTR)(node)))
#define AVL_TREE_PARENT_NODE(node)	((AVL_TREE_PTR)GetTreeNodeParent((BINARY_TREE_PTR)(node), NULL))

#define AVL_TREE_LEFT_LIST(node)	((node)->bintree.left)
#define AVL_TREE_RIGHT_LIST(node)	((node)->bintree.right)

#endif
