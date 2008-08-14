/*! 
	\file 	ds/binary_tree.h
	\brief 	Binary tree implementation
*/

#ifndef BINARY_TREE__H
#define BINARY_TREE__H

#include "list.h"
#include "ace.h"

typedef enum 
{
	NO_LIST			=	-1,
	LEFT_TREE_LIST	=	0,
	RIGHT_TREE_LIST	=	1
}TREE_LIST_TYPE;

/*! Binary tree data structure
*/
typedef struct binary_tree 
{
	LIST left;
	LIST right;
	
	LIST sibling[0];
}BINARY_TREE, * BINARY_TREE_PTR;

/*! binary tree with duplicate keys*/
typedef struct binary_tree_d
{
	BINARY_TREE bintree;
	LIST		sibling;
}BINARY_TREE_D, * BINARY_TREE_D_PTR;

#define BIN_TREE_NODE(name, function_ptr) BINARY_TREE name = {NULL, NULL, function_ptr}

/*! Returns left tree node of a binary tree*/
#define TREE_LEFT_NODE(  node ) 		( STRUCT_ADDRESS_FROM_MEMBER( (((unsigned long)(node)->left.next)  & ~1), BINARY_TREE, left ) ) 
/*! Returns right tree node of a binary tree*/
#define TREE_RIGHT_NODE( node )			( STRUCT_ADDRESS_FROM_MEMBER( (((unsigned long)(node)->right.next) & ~1), BINARY_TREE, right) )

/*! Returns left parent of a binary tree*/
#define TREE_LEFT_PARENT(  node )   	( STRUCT_ADDRESS_FROM_MEMBER( (((unsigned long)(node)->left.prev)  & ~1), BINARY_TREE, left ) ) 
/*! Returns right parent of a binary tree*/
#define TREE_RIGHT_PARENT( node )		( STRUCT_ADDRESS_FROM_MEMBER( (((unsigned long)(node)->right.prev) & ~1), BINARY_TREE, right) )

/*! Checks whether the given tree list is end node or not*/
#define IS_TREE_LIST_END(list)			( (int)(((unsigned long)((list)->next)) & 1) )
/*! checks whether the left node is end node or not*/
#define IS_END_OF_LEFT_LIST(node) 		( IS_TREE_LIST_END(&((node)->left)) )
/*! checks whether the right node is end node or not*/
#define IS_END_OF_RIGHT_LIST(node) 		( IS_TREE_LIST_END(&((node)->right)) )

#ifdef __cplusplus
    extern "C" {
#endif

BINARY_TREE_PTR SearchBinaryTree(BINARY_TREE_PTR root, BINARY_TREE_PTR search_node, COMPARISION_RESULT (*fnCompare)(BINARY_TREE_PTR, BINARY_TREE_PTR));
int InsertNodeIntoBinaryTree(BINARY_TREE_PTR * root_ptr, BINARY_TREE_PTR new_node, int duplicates_allowed, COMPARISION_RESULT (*fnCompare)(BINARY_TREE_PTR, BINARY_TREE_PTR));
int RemoveNodeFromBinaryTree(BINARY_TREE_PTR node, BINARY_TREE_PTR * leaf_node, BINARY_TREE_PTR * root_ptr, int duplicates_allowed, BINARY_TREE_PTR * sibling_ptr, COMPARISION_RESULT (*fnCompare)(BINARY_TREE_PTR, BINARY_TREE_PTR));

BINARY_TREE_PTR InitBinaryTreeNode(BINARY_TREE_PTR node, int duplicates_allowed);
BINARY_TREE_PTR GetTreeNodeParent(BINARY_TREE_PTR node, TREE_LIST_TYPE * list_type);

void RotateRight(BINARY_TREE_PTR node, BINARY_TREE_PTR *root_ptr);
void RotateLeft(BINARY_TREE_PTR  node, BINARY_TREE_PTR *root_ptr);

#ifdef __cplusplus
	}
#endif


#endif
