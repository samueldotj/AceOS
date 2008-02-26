/*!
  \file		avl_tree.h
  \author	DilipSimha N M
  \version 	3.0
  \date	
  			Created:
  			Last modified: Mon Feb 25, 2008  12:29PM
  \brief	
*/
#ifndef _AVL_TREE_H_
#define _AVL_TREE_H_

#include <binary_tree.h>

struct AVLTree;

/*structures go here*/
struct AVLTree {
	BINARY_TREE	bintree;
    int			height;
};
typedef struct AVLTree  AVL_TREE, *AVL_TREE_PTR;

AVL_TREE_PTR SearchAvlTree(AVL_TREE_PTR start, AVL_TREE_PTR search_node);
int InsertNodeIntoAvlTree(AVL_TREE_PTR *root, AVL_TREE_PTR new_node);
int RemoveNodeFromAvlTree(AVL_TREE_PTR *root, AVL_TREE_PTR node);


#define AVL_TREE_LEFT_NODE(node)	(AVL_TREE_PTR)TREE_LEFT_NODE((BINARY_TREE_PTR)(node))
#define AVL_TREE_RIGHT_NODE(node)	(AVL_TREE_PTR)TREE_RIGHT_NODE((BINARY_TREE_PTR)(node))
#define AVL_TREE_PARENT_NODE(node)	(AVL_TREE_PTR)GetTreeNodeParent((BINARY_TREE_PTR)(node), NULL)

#endif
