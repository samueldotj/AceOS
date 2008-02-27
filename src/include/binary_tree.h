/*! 
	\file binary_tree.h
	\brief Binary tree implementation
	\author Samuel
	\date 
		Created: 04-Feb-2008 18:24
		Last modified: Wed Feb 27, 2008  11:25AM
*/

#ifndef BINARY_TREE__H
#define BINARY_TREE__H

#include "list.h"

typedef enum 
{
	LESS_THAN=-1,
	EQUAL=0,
	GREATHER_THAN=1
}COMPARISION_RESULT;


typedef enum 
{
	NO_LIST=-1,
	LEFT_TREE_LIST=0,
	RIGHT_TREE_LIST=1
}TREE_LIST_TYPE;

/*! Binary tree data structure
*/
struct binary_tree {
	LIST left;
	LIST right;
	
	COMPARISION_RESULT (*fnCompareKey)(struct binary_tree * , struct binary_tree *);
};
typedef struct binary_tree BINARY_TREE, * BINARY_TREE_PTR;

#define BIN_TREE_NODE(name, function_ptr) BINARY_TREE name = {NULL, NULL, function_ptr}

#define TREE_LEFT_NODE(  node ) ( (BINARY_TREE_PTR)( STRUCT_FROM_MEMBER(BINARY_TREE_PTR, left,  (((unsigned long)(node)->left.next) & ~1) )  ) )
#define TREE_RIGHT_NODE( node )	( (BINARY_TREE_PTR)( STRUCT_FROM_MEMBER(BINARY_TREE_PTR, right, (((unsigned long)(node)->right.next) & ~1)) ) )

#define TREE_LEFT_PARENT(  node )   ( (BINARY_TREE_PTR)( STRUCT_FROM_MEMBER(BINARY_TREE_PTR, left,  (((unsigned long)(node)->left.prev) & ~1) )  ) )
#define TREE_RIGHT_PARENT( node )	( (BINARY_TREE_PTR)( STRUCT_FROM_MEMBER(BINARY_TREE_PTR, right, (((unsigned long)(node)->right.prev) & ~1)) ) )
#define IS_TREE_LIST_END(list)		( (int)(((unsigned long)((list)->next)) & 1) )

#define IS_END_OF_LEFT_LIST(node) 	( IS_TREE_LIST_END(&((node)->left)) )
#define IS_END_OF_RIGHT_LIST(node) 	( IS_TREE_LIST_END(&((node)->right)) )

#ifdef __cplusplus
    extern "C" {
#endif

BINARY_TREE_PTR SearchBinaryTree(BINARY_TREE_PTR root, BINARY_TREE_PTR search_node);
int InsertNodeIntoBinaryTree(BINARY_TREE_PTR root, BINARY_TREE_PTR new_node);
void RemoveNodeFromBinaryTree(BINARY_TREE_PTR node, BINARY_TREE_PTR * leaf_node, BINARY_TREE_PTR * root_ptr);

BINARY_TREE_PTR InitBinaryTreeNode(BINARY_TREE_PTR node, COMPARISION_RESULT (*compare_function_ptr)(struct binary_tree * , struct binary_tree *));
BINARY_TREE_PTR GetTreeNodeParent(BINARY_TREE_PTR node, TREE_LIST_TYPE * list_type);

void UnlinkTreeList(LIST_PTR list_node);
void LinkTwoTreeLists( LIST_PTR list1head, LIST_PTR list2head );
void ReplaceTreeListNode(LIST_PTR old_node, LIST_PTR new_node);
	
#ifdef __cplusplus
	}
#endif


#endif
