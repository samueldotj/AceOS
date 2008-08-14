/*!
  \file		ds/avl_tree.h
  \brief	Generic AVL Tree Implementation
*/
#ifndef _AVL_TREE_H_
#define _AVL_TREE_H_

#include "binary_tree.h"

/*! avl tree without duplicate keys*/
typedef struct avl_tree 
{
	/*! height of the node*/
	int			height;
	/*! low level binary tree*/
	BINARY_TREE	bintree;
}AVL_TREE, *AVL_TREE_PTR;
/*! avl tree with duplicate keys*/
typedef struct avl_tree_d
{
	/*low level avl tree*/
	AVL_TREE	avltree;
	/*duplicate nodes with the same key*/
	LIST		sibling;
}AVL_TREE_D, *AVL_TREE_D_PTR;

AVL_TREE_PTR InitAvlTreeNode(AVL_TREE_PTR node, int duplicates_allowed);
AVL_TREE_PTR SearchAvlTree(AVL_TREE_PTR start, AVL_TREE_PTR search_node, void * fnCompare);
int InsertNodeIntoAvlTree(AVL_TREE_PTR *root, AVL_TREE_PTR new_node, int duplicates_allowed, void * fnCompare);
int RemoveNodeFromAvlTree(AVL_TREE_PTR *root, AVL_TREE_PTR node, int duplicates_allowed, void * fnCompare);


#define AVL_TREE_LEFT_NODE(node)	( STRUCT_ADDRESS_FROM_MEMBER( TREE_LEFT_NODE(&(node)->bintree), AVL_TREE, bintree ) )
#define AVL_TREE_RIGHT_NODE(node)	( STRUCT_ADDRESS_FROM_MEMBER( TREE_RIGHT_NODE(&(node)->bintree), AVL_TREE, bintree ) )
#define AVL_TREE_PARENT_NODE(node)	( GetAvlTreeNodeParent(node) )
			
#define AVL_TREE_LEFT_LIST(node)	((node)->bintree.left)
#define AVL_TREE_RIGHT_LIST(node)	((node)->bintree.right)

#define IS_AVL_TREE_LEFT_LIST_END(node)		(IS_TREE_LIST_END( &AVL_TREE_LEFT_LIST(node)) )
#define IS_AVL_TREE_RIGHT_LIST_END(node)	(IS_TREE_LIST_END( &AVL_TREE_RIGHT_LIST(node)) )

#endif
