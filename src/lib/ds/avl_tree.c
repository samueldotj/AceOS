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
#include <stdio.h>
#include <stdlib.h>

#define GET_AVL_TREE_HEIGHTS( node, left_height, right_height )	\
	if ( !IS_END_OF_LEFT_LIST( (node) ) )						\				
		left_height = AVL_TREE_LEFT_NODE( (node) )->height;		\
	if ( !IS_END_OF_RIGHT_LIST( (node) ) )						\
		right_height = AVL_TREE_RIGHT_NODE( (node) )->height;	

#define RECALCULATE_HEIGHT( node ) (node)->height = RecalculateAvlTreeHeight( node )

/*declarations go here*/
AVL_TREE_PTR InitAvlTreeNode(AVL_TREE_PTR node, void *compare_function_ptr);
static int BalanceAvlTree(AVL_TREE_PTR start_node, AVL_TREE_PTR *root);
static int GetAvlTreeBalanceFactor(AVL_TREE_PTR node);
static void RightRotateAvlTree(AVL_TREE_PTR *node, AVL_TREE_PTR *root);
static void LeftRotateAvlTree(AVL_TREE_PTR *node, AVL_TREE_PTR *root);
static void single_rotate_right_avltree(AVL_TREE_PTR *node, AVL_TREE_PTR *root);
static void single_rotate_left_avltree(AVL_TREE_PTR *node, AVL_TREE_PTR *root);
static void double_rotate_left_right(AVL_TREE_PTR *start, AVL_TREE_PTR *root);
static void double_rotate_right_left(AVL_TREE_PTR *start, AVL_TREE_PTR *root);


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
	Theory:
		AVL tree uses binary tree internally.
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

	/* Balance the entire tree. 
	 * Parent will always be balanced; So get grand-parent and start balancing from there.
	 */
	parent = AVL_TREE_PARENT_NODE(new_node);
	if (parent == *root)
	{ 
		/*parent is the root, so tree is balanced*/
		return 0;
	}

	RECALCULATE_HEIGHT(parent);
	
	grand_parent = AVL_TREE_PARENT_NODE(parent);
	if (grand_parent == parent) { /*No grand parent, so tree is balanced*/
		return 0;
	}
	
	return BalanceAvlTree(parent, root);
}

static int RecalculateAvlTreeHeight(AVL_TREE_PTR node)
{
	int left_height=0, right_height=0;
	GET_AVL_TREE_HEIGHTS( node, left_height, right_height );
	return MAX( left_height, right_height ) + 1;
}

static int GetAvlTreeBalanceFactor(AVL_TREE_PTR node)
{
	int left_height=-1, right_height=-1, balance_factor;
	GET_AVL_TREE_HEIGHTS( node, left_height, right_height );

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

void BalanceAvlTree(AVL_TREE_PTR start_node, AVL_TREE_PTR *root)
{
	int balance_factor;
	
	RECALCULATE_HEIGHT(start_node);
	balance_factor = GetAvlTreeBalanceFactor(start_node);

	switch(balance_factor) 
	{
		case 0:
		case 1:
		case -1: /*tree is balanced*/
			return 0;
		case 2: /*heavy on right*/
			LeftRotateAvlTree(&start_node, root);
			break;
		case -2: /*heavy on left*/
			RightRotateAvlTree(&start_node, root);
			break;
		default:
			assert(0);
	}

	parent = AVL_TREE_PARENT_NODE(start_node);
	/* continue till root */
	if (parent != start_node) 
		BalanceAvlTree(parent, root);
}


/* decide on single or double right rotation*/
void RightRotateAvlTree(AVL_TREE_PTR *node, AVL_TREE_PTR *root)
{
	/* No need to check if node is leaf node or not, because we come here only when 
	 * left subtree is heavy.
	 */
	int balance_factor = GetAvlTreeBalanceFactor(AVL_TREE_LEFT_NODE(*node));
	switch(balance_factor) {
		case 1: /*double rotate*/
			double_rotate_left_right(node, root); 
			break;
		case -1: /*single rotate*/
			single_rotate_right_avltree(node, root);
			break;
		default:  /*not possible....*/
			assert(0);
			break;
	}
	return;
}

/* decide on single or double left rotation*/
void LeftRotateAvlTree(AVL_TREE_PTR *node, AVL_TREE_PTR *root)
{
	/* No need to check if node is leaf node or not, because we come here only when 
	 * right subtree is heavy.
	 */
	int balance_factor = GetAvlTreeBalanceFactor(AVL_TREE_RIGHT_NODE(*node));

	switch(balance_factor) {
		case -1: /*double rotate*/
			double_rotate_right_left(node, root);
			break;
		case 0:
		case 1: /*single rotate*/
			single_rotate_left_avltree(node, root);
			break;
		default:  /*not possible....*/
			assert(0);
			break;
	}
	return;
}

void single_rotate_right_avltree(AVL_TREE_PTR *node, AVL_TREE_PTR *root)
{
	/*rotate right*/
	AVL_TREE_PTR temp;

	temp = AVL_TREE_LEFT_NODE(*node);
	AVL_TREE_LEFT_NODE(*node) = AVL_TREE_RIGHT_NODE(temp);
	AVL_TREE_RIGHT_NODE(temp) = *node;

	if(*node == *root) {
		*root = temp;
	}
	(*node) = temp;

	/*calculate the new heights*/
	temp = AVL_TREE_RIGHT_NODE(*node);
	RECALCULATE_HEIGHT( temp );
	
	RECALCULATE_HEIGHT( *node);
	return;
}

/*! Performs a left rotation, rooted at *node_ptr
	and updates the root pointer if neccesary
	and adjust the heights
	
	Algo for left rotation
		parent = node
		child = node->right
		1) Remove parent from the right list
		2) Link child's "left child" as parents right child
		3) Link parent as "left child" of child.
*/
void single_rotate_left_avltree(AVL_TREE_PTR *node_ptr, AVL_TREE_PTR *root_ptr)
{
	AVL_TREE_PTR parent, child;
	
	parent = *node_ptr;
	child = AVL_TREE_RIGHT_NODE(parent);
	
	//Remove parent from the right list
	UnlinkTreeList(&parent->right);
	
	//Link child's "left child" as parents right child
	if ( !IS_END_OF_LEFT_LIST( child ) )
	{
		LIST_PTR left = &child->left.next;
		UnlinkTreeList(&child->left);
		LinkTwoTreeLists( &parent->right,  left);
	}
	//Link parent as "left child" of child.
	LinkTwoTreeLists( &child->left, &parent->right );
	
	//updates the root pointer if neccesary
	if ( *node_ptr = *root_ptr )
		*root_ptr = child;
	
	/*calculate the new heights*/
	RECALCULATE_HEIGHT(parent);
	RECALCULATE_HEIGHT(child);
	
	return;
}

void double_rotate_left_right(AVL_TREE_PTR *start, AVL_TREE_PTR *root)
{
	/*First rotate left*/
	AVL_TREE_PTR temp, temp_left;

	temp_left = AVL_TREE_LEFT_NODE(*start);

	temp = AVL_TREE_RIGHT_NODE(temp_left);
	AVL_TREE_RIGHT_NODE(temp_left) = AVL_TREE_LEFT_NODE(temp);
	AVL_TREE_LEFT_NODE(temp) = temp_left;

	AVL_TREE_LEFT_NODE(*start) = temp;

	/*calculate the new heights*/
	RECALCULATE_HEIGHT(temp_left);
	
	/*now rotate right*/
	AVL_TREE_LEFT_NODE(*start) = AVL_TREE_RIGHT_NODE(temp);
	AVL_TREE_RIGHT_NODE(temp) = (*start);

	if(*start == *root) {
		*root = temp;
	}
	(*start) = temp;

	/*calculate the new heights*/
	temp = AVL_TREE_RIGHT_NODE(*start);
	RECALCULATE_HEIGHT(temp)
	
	RECALCULATE_HEIGHT(*start);
	return;
}

void double_rotate_right_left(AVL_TREE_PTR *start, AVL_TREE_PTR *root)
{
	/*First rotate right*/
	AVL_TREE_PTR temp, temp_right;

	temp_right = AVL_TREE_RIGHT_NODE(*start);

	temp = AVL_TREE_LEFT_NODE(temp_right);
	AVL_TREE_LEFT_NODE(temp_right) = AVL_TREE_RIGHT_NODE(temp);
	AVL_TREE_RIGHT_NODE(temp) = temp_right;

	AVL_TREE_RIGHT_NODE(*start) = temp;

	/*calculate the new heights*/
	RECALCULATE_HEIGHT( temp_right );
	
	/*now rotate left*/
	AVL_TREE_RIGHT_NODE(*start) = AVL_TREE_LEFT_NODE(temp);
	AVL_TREE_LEFT_NODE(temp) = (*start);

	if (*start == *root) {
		*root = temp;
	}
	(*start) = temp;

	/*calculate the new heights*/
	temp = AVL_TREE_LEFT_NODE(*start);
	RECALCULATE_HEIGHT( temp_right );
	RECALCULATE_HEIGHT( *start );
	return;
}

