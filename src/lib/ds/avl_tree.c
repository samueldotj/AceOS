/*
 * File: avl_tree.c
 *
 * Copyright 2007, HP.  All rights reserved.
 *
 * Notes:
 *
 * Author:  DilipSimha.N.M
 *
 */

#include <avl_tree.h>
#include <ace.h>

/*This will only return heights of non-leaf, left and right nodes*/

#define GET_AVL_TREE_HEIGHTS( node, left_height, right_height )					\
	left_height = right_height = -1;							\
	if ( !IS_END_OF_LEFT_LIST( (BINARY_TREE_PTR)(node) ) )		\
		left_height = AVL_TREE_LEFT_NODE( (node) )->height;						\
	if ( !IS_END_OF_RIGHT_LIST( (BINARY_TREE_PTR)(node) ) )		\
		right_height = AVL_TREE_RIGHT_NODE( (node) )->height;	\

#define RECALCULATE_HEIGHT( node ) (node)->height = RecalculateAvlTreeHeight( node )

/*declarations go here*/
static int BalanceAvlTree(AVL_TREE_PTR start_node, AVL_TREE_PTR *root_ptr);
static int GetAvlTreeBalanceFactor(AVL_TREE_PTR node);
static int RecalculateAvlTreeHeight(AVL_TREE_PTR node);

static void RightRotateAvlTree(AVL_TREE_PTR node, AVL_TREE_PTR *root_ptr);
static void LeftRotateAvlTree(AVL_TREE_PTR  node, AVL_TREE_PTR *root_ptr);


/*! Initializes the avl tree structure.
 */
AVL_TREE_PTR InitAvlTreeNode(AVL_TREE_PTR node, void *compare_function_ptr)
{
	InitBinaryTreeNode((BINARY_TREE_PTR)node, compare_function_ptr);
	node->height=0;
	
	return node;
}

/*! Searches the AVL tree and returns the identified node
		Searching AVL tree is same as searching a binary tree
*/
AVL_TREE_PTR SearchAvlTree(AVL_TREE_PTR start, AVL_TREE_PTR search_node)
{
	return (AVL_TREE_PTR)SearchBinaryTree((BINARY_TREE_PTR)start, (BINARY_TREE_PTR)search_node);
}

/*! Removes the given node from the AVL tree.
	Algo:
		1) Remove the node using binary tree function
		2) Balance the AVL Tree
*/
int RemoveNodeFromAvlTree(AVL_TREE_PTR *root, AVL_TREE_PTR node)
{
	AVL_TREE_PTR parent;
	RemoveNodeFromBinaryTree((BINARY_TREE_PTR)node, (BINARY_TREE_PTR*)(&parent), (BINARY_TREE_PTR*)root);
	/*update the height of this node*/
	RECALCULATE_HEIGHT(parent);

	/*now check the balance_factor*/
	return BalanceAvlTree(parent, root);
}

/*! Inserts an already created node into the avl tree.
	Return values:
		0    SUCCESS
		-1   FAIL (Duplicate value, not inserted)
		-2   Balance factor error
	Algorithm:
		1) Call Binary tree functions to insert node. 
		2) Balance the tree and update height value.
 */
int InsertNodeIntoAvlTree(AVL_TREE_PTR *root, AVL_TREE_PTR new_node)
{
	AVL_TREE_PTR parent, grand_parent;

	if ( InsertNodeIntoBinaryTree( (BINARY_TREE_PTR)(*root), (BINARY_TREE_PTR)new_node)) 
	{
		/*Failure: Duplicate value already exists, so return -1 */
		return -1;
	}
	printf("node inserted\n");

	if (new_node == *root) {
		return 0;
	}
	/* Balance the entire tree. 
	 * Parent will always be balanced; So get grand-parent and start balancing from there.
	 */
	parent = AVL_TREE_PARENT_NODE(new_node);
	if (parent == *root)
	{
		/* 
		 * parent is the root, so tree is balanced
		 * Only update the height.
		 */
		RECALCULATE_HEIGHT(parent);
		return 0;
	}
	RECALCULATE_HEIGHT(parent);
	grand_parent = AVL_TREE_PARENT_NODE(parent);
	if (grand_parent == parent) 
	{
		printf("No grandparent\n");
		/*No grand parent, so tree is balanced*/
		return 0;
	}
	printf("calling balance tree for node height = %d\n", grand_parent->height);
	return BalanceAvlTree(grand_parent, root);
}

/*! Recalculates the AVL Tree height and returns the value
*/
static int RecalculateAvlTreeHeight(AVL_TREE_PTR node)
{
	int left_height, right_height;
	GET_AVL_TREE_HEIGHTS( node, left_height, right_height );
	printf("recalculate height: left height = %d and rigth_height = %d\n", left_height, right_height);
	return MAX( left_height, right_height ) + 1;
}

/*! Returns the balance factor for the given AVL tree node
*/
static int GetAvlTreeBalanceFactor(AVL_TREE_PTR node)
{
	int left_height, right_height;
	GET_AVL_TREE_HEIGHTS( node, left_height, right_height );
	printf("get balance factor: left_height=%d and right_height=%d\n", left_height, right_height);

	return right_height-left_height;
}

/*! Adjusts the balance factor of the avl tree.
	Theory:
		Recursively go from leaf node to parent, balancing the tree.
		Choose  left/right rotations based on balance factor.
	Algorithm:
		1) Get the balance factor. 
		2) Based on it's value, call left or right rotations.
		3) If stable, then avl tree is balanced, so simply return.
 */
static int BalanceAvlTree(AVL_TREE_PTR start_node, AVL_TREE_PTR *root_ptr)
{
	int balance_factor;
	AVL_TREE_PTR parent;

	assert(start_node != NULL);

	RECALCULATE_HEIGHT(start_node);
	printf("starting recalculated height = %d\n", start_node->height);
	balance_factor = GetAvlTreeBalanceFactor(start_node);
	assert( balance_factor >= -2 &&  balance_factor <= 2 );

	switch(balance_factor) 
	{
		case 0:
		case 1:
		case -1: /*tree is balanced*/
			printf("tree is balanced\n");
			break;
		case 2: /*heavy on right*/
			printf("left rotating at node\n");
			LeftRotateAvlTree(start_node, root_ptr);
			break;
		case -2: /*heavy on left*/
			printf("Right rotating at node\n");
			RightRotateAvlTree(start_node, root_ptr);
			break;
	}

	if (start_node == *root_ptr) {
		return 0;
	}

	printf("getting parent node\n");
	parent = AVL_TREE_PARENT_NODE(start_node);
	/* start_node is now a child and hence it's parent is also balanced*/
	/* continue recursion till root */
	printf("recursive call....\n");
	BalanceAvlTree(parent, root_ptr);
	return 0;
}

/*! Single or double right rotates the AVL tree on the node to balance
*/
void RightRotateAvlTree(AVL_TREE_PTR node, AVL_TREE_PTR *root_ptr)
{
	AVL_TREE_PTR child = AVL_TREE_LEFT_NODE(node);
	int balance_factor = GetAvlTreeBalanceFactor(child);
	/*only -1,0 or 1 balance is expected*/
	assert( balance_factor<=1 && balance_factor>=-1 );
	
	/*check for double rotate*/
	if ( balance_factor == 1 )
		RotateLeft( &child->bintree, (BINARY_TREE_PTR *) root_ptr);

	/*single right rotate*/
	RotateRight( &node->bintree, (BINARY_TREE_PTR *) root_ptr);
	
	/*calculate the new heights*/
	RECALCULATE_HEIGHT(node);
	RECALCULATE_HEIGHT(child);
}

/*! Single or double left rotates the AVL tree on the node to balance
*/
void LeftRotateAvlTree(AVL_TREE_PTR node, AVL_TREE_PTR *root_ptr)
{
	AVL_TREE_PTR child = AVL_TREE_RIGHT_NODE(node);
	int balance_factor = GetAvlTreeBalanceFactor(child);
	printf("inside left rotation: balance factor = %d\n", balance_factor);
	/*only -1,0 or 1 balance is expected*/
	assert( balance_factor<=1 && balance_factor>=-1 );
	
	/*check for double rotate*/
	if ( balance_factor == -1 )
		RotateRight( &child->bintree, (BINARY_TREE_PTR *)root_ptr);
	
	/*single rotate*/
	RotateLeft( (BINARY_TREE_PTR)node, (BINARY_TREE_PTR *)root_ptr);
	printf("rotated left\n");
	
	/*calculate the new heights*/
	RECALCULATE_HEIGHT(node);
	RECALCULATE_HEIGHT(child);
}
