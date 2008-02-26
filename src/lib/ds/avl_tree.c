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

/*MACROS*/
#define AVL_MAX(a,b) (((a) > (b)) ? (a):(b))

/*extern declarations come here*/

/*declarations go here*/
AVL_TREE_PTR InitAvlTreeNode(AVL_TREE_PTR node, void *compare_function_ptr);
static int get_height_avltree(AVL_TREE_PTR node);
static int adjust_balanace_factor_avltree(AVL_TREE_PTR start_node, AVL_TREE_PTR *root);
static int get_balance_factor_avltree(AVL_TREE_PTR node);
static void right_rotate_decide(AVL_TREE_PTR *node, AVL_TREE_PTR *root);
static void left_rotate_decide(AVL_TREE_PTR *node, AVL_TREE_PTR *root);
static void single_rotate_right_avltree(AVL_TREE_PTR *node, AVL_TREE_PTR *root);
static void single_rotate_left_avltree(AVL_TREE_PTR *node, AVL_TREE_PTR *root);
static void double_rotate_left_right(AVL_TREE_PTR *start, AVL_TREE_PTR *root);
static void double_rotate_right_left(AVL_TREE_PTR *start, AVL_TREE_PTR *root);

/*functions start here*/


/*! Initializes the avl tree structure.

 */
AVL_TREE_PTR InitAvlTreeNode(AVL_TREE_PTR node, void *compare_function_ptr)
{
	InitBinaryTreeNode((BINARY_TREE_PTR)node, compare_function_ptr);
	return node;
}


int get_height_avltree(AVL_TREE_PTR node)
{
	if(!node) {
		 return -1;
	}

	return node->height;
}


/*! Inserts an already created node into the avl tree.
  Return values:
  0    SUCCESS
  -1   FAIL (Duplicate value, not inserted)
  -2   Balance factor error
Theory:
AVL tree uses binary tree internally.
Algorithm:
1) Call Binary tree functions to indert node. 
2) Balance the tree and update height value.
 */

int InsertNodeIntoAvlTree(AVL_TREE_PTR *root, AVL_TREE_PTR new_node)
{
	AVL_TREE_PTR parent, grand_parent;

	if (InsertNodeIntoBinaryTree((BINARY_TREE_PTR)(*root), (BINARY_TREE_PTR)new_node)) {
		/*
		 * Failure: Duplicate value already exists, so return -1.
		 */
		return -1;
	}
	new_node->height = 0;

	/*
	 * Balance the entire tree.
	 * Parent will always be balanced.
	 * So get grand-parent and start balancing from there.
	 */
	parent = AVL_TREE_PARENT_NODE(new_node);
	if (parent == *root) { /*parent is the root, so tree is balanced*/
		return 0;
	}

	parent->height = AVL_MAX(get_height_avltree(AVL_TREE_LEFT_NODE(parent)),
            get_height_avltree(AVL_TREE_RIGHT_NODE(parent))) + 1; 
	
	grand_parent = AVL_TREE_PARENT_NODE(parent);
	if (grand_parent == parent) { /*No grand parent, so tree is balanced*/
		return 0;
	}
	
	return adjust_balanace_factor_avltree(parent, root);
}

/*! Adjusts the balance factor of the avl tree.
  Return values:
  0    SUCCESS
  -2   Invalid value for Balance factor.
Theory:
Recursively go from leaf node to parent, balancing the tree.
Choose  left/right rotations based on balance factor.
Algorithm:
1) Get the balance factor. 
2) Based on it's value, call left or right rotations.
3) If stable, then avl tree is balanced, so simply return.
 */

int adjust_balanace_factor_avltree(AVL_TREE_PTR start_node, AVL_TREE_PTR *root)
{
	int balance_factor;
	AVL_TREE_PTR parent, left_node, right_node;
	
	if (!IS_END_OF_LEFT_LIST( (BINARY_TREE_PTR)(start_node) )) {
		left_node = AVL_TREE_LEFT_NODE(start_node);
	} else {
		left_node = NULL;
	}

	if (!IS_END_OF_RIGHT_LIST( (BINARY_TREE_PTR)(start_node) )) {
		right_node = AVL_TREE_RIGHT_NODE(start_node);
	} else {
		right_node = NULL;
	}

	start_node->height = AVL_MAX(get_height_avltree(left_node),
            get_height_avltree(right_node)) + 1;

	balance_factor = get_balance_factor_avltree(start_node);
	/*rotate tree*/
	switch(balance_factor) {
		case 0:
		case 1:
		case -1: /*tree is balanced*/
			return 0;
		case 2: /*heavy on right*/
			left_rotate_decide(&start_node, root);
			break;
		case -2: /*heavy on left*/
			right_rotate_decide(&start_node, root);
			break;
		default:  /*not possible....*/
			fprintf(stderr, "FATAL Error: Not possible to \
					have balance_factor=%d\n",
					balance_factor);
			return -2;
	}

	parent = AVL_TREE_PARENT_NODE(start_node);
	if (parent != start_node) { /* continue till root */
		return adjust_balanace_factor_avltree(parent, root);
	}

	return 0; /*SUCCESS*/
}

int get_balance_factor_avltree(AVL_TREE_PTR node)
{
	AVL_TREE_PTR left_node, right_node;
	int balance_factor;

	if (!IS_END_OF_LEFT_LIST( (BINARY_TREE_PTR)(node) )) {
		left_node = AVL_TREE_LEFT_NODE(node);
	} else {
		left_node = NULL;
	}

	if (!IS_END_OF_RIGHT_LIST( (BINARY_TREE_PTR)(node) )) {
		right_node = AVL_TREE_RIGHT_NODE(node);
	} else {
		right_node = NULL;
	}

	balance_factor = get_height_avltree(right_node) -
		get_height_avltree(left_node);

	return balance_factor;
}


/* decide on single or double right rotation*/
void right_rotate_decide(AVL_TREE_PTR *node, AVL_TREE_PTR *root)
{
	/* No need to check if node is leaf node or not, because we come here only when 
	 * left subtree is heavy.
	 */
	int balance_factor = get_balance_factor_avltree(AVL_TREE_LEFT_NODE(*node));
	switch(balance_factor) {
		case 1: /*double rotate*/
			double_rotate_left_right(node, root); 
			break;
		case -1: /*single rotate*/
			single_rotate_right_avltree(node, root);
			break;
		default:  /*not possible....*/
			fprintf(stderr, "FATAL Error: In second level RR, \
					Not possible to have balance_factor=%d\n",
					balance_factor);
			break;
	}
	return;
}

/* decide on single or double left rotation*/
void left_rotate_decide(AVL_TREE_PTR *node, AVL_TREE_PTR *root)
{
	/* No need to check if node is leaf node or not, because we come here only when 
	 * right subtree is heavy.
	 */
	int balance_factor = get_balance_factor_avltree(AVL_TREE_RIGHT_NODE(*node));

	switch(balance_factor) {
		case -1: /*double rotate*/
			double_rotate_right_left(node, root);
			break;
		case 0:
		case 1: /*single rotate*/
			single_rotate_left_avltree(node, root);
			break;
		default:  /*not possible....*/
			fprintf(stderr, "FATAL Error: In second level LR, \
					Not possible to have balance_factor=%d\n",
					balance_factor);
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
	temp->height =  AVL_MAX(get_height_avltree(AVL_TREE_LEFT_NODE(temp)),
			get_height_avltree(AVL_TREE_RIGHT_NODE(temp))) + 1;

	(*node)->height =  AVL_MAX(get_height_avltree(AVL_TREE_LEFT_NODE(*node)),
			temp->height) + 1;
	return;
}

void single_rotate_left_avltree(AVL_TREE_PTR *node_ptr, AVL_TREE_PTR *root_ptr)
{
	AVL_TREE_PTR parent, child;
	
	parent = *node_ptr;
	child = AVL_TREE_RIGHT_NODE(parent);
	
	UnlinkTreeList(&parent->right);
	if ( !IS_END_OF_LEFT_LIST( child ) )
	{
		LIST_PTR left = &child->left.next;
		UnlinkTreeList(&child->left);
		LinkTwoTreeLists( &parent->right,  left);
	}
	LinkTwoTreeLists( &parent->right, &child->left);
	
	if ( *node_ptr = *root_ptr )
		*root_ptr = child;
	
	/*rotate left*/
	AVL_TREE_PTR temp;

	temp = AVL_TREE_RIGHT_NODE(*node);
	AVL_TREE_RIGHT_NODE(*node) = AVL_TREE_LEFT_NODE(temp);
	AVL_TREE_LEFT_NODE(temp) = (*node);

	if(*node == *root) {
		*root = temp;
	}
	(*node) = temp;

	/*calculate the new heights*/
	temp = AVL_TREE_LEFT_NODE(*node);
	temp->height =  AVL_MAX(get_height_avltree(AVL_TREE_LEFT_NODE(temp)),
			get_height_avltree(AVL_TREE_RIGHT_NODE(temp))) + 1;

	(*node)->height =  AVL_MAX(temp->height, 
		get_height_avltree(AVL_TREE_RIGHT_NODE(*node))) + 1;
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
	temp_left->height =  AVL_MAX(get_height_avltree(AVL_TREE_LEFT_NODE(temp_left)),
			get_height_avltree(AVL_TREE_RIGHT_NODE(temp_left))) + 1;

	/*now rotate right*/
	AVL_TREE_LEFT_NODE(*start) = AVL_TREE_RIGHT_NODE(temp);
	AVL_TREE_RIGHT_NODE(temp) = (*start);

	if(*start == *root) {
		*root = temp;
	}
	(*start) = temp;

	/*calculate the new heights*/
	temp = AVL_TREE_RIGHT_NODE(*start);
	temp->height =  AVL_MAX(get_height_avltree(AVL_TREE_LEFT_NODE(temp)),
			get_height_avltree(AVL_TREE_RIGHT_NODE(temp))) + 1;

	(*start)->height =  AVL_MAX(get_height_avltree(AVL_TREE_LEFT_NODE(*start)),
			temp->height) + 1;
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
	temp_right->height =  AVL_MAX(get_height_avltree(AVL_TREE_LEFT_NODE(temp_right)),
			get_height_avltree(AVL_TREE_RIGHT_NODE(temp_right))) + 1;

	/*now rotate left*/
	AVL_TREE_RIGHT_NODE(*start) = AVL_TREE_LEFT_NODE(temp);
	AVL_TREE_LEFT_NODE(temp) = (*start);

	if (*start == *root) {
		*root = temp;
	}
	(*start) = temp;

	/*calculate the new heights*/
	temp = AVL_TREE_LEFT_NODE(*start);
	temp->height =  AVL_MAX(get_height_avltree(AVL_TREE_LEFT_NODE(temp)),
			get_height_avltree(AVL_TREE_RIGHT_NODE(temp))) + 1;

	(*start)->height =  AVL_MAX(temp->height, get_height_avltree(AVL_TREE_RIGHT_NODE(*start)))
		+ 1;
	return;
}

int RemoveNodeFromAvlTree(AVL_TREE_PTR *root, AVL_TREE_PTR node)
{
	AVL_TREE_PTR parent;
	RemoveNodeFromBinaryTree((BINARY_TREE_PTR)node, (BINARY_TREE_PTR*)(&parent), (BINARY_TREE_PTR*)root);
	/*update the height of this node*/
	parent->height = AVL_MAX(get_height_avltree(AVL_TREE_LEFT_NODE(parent)),
			get_height_avltree(AVL_TREE_RIGHT_NODE(parent))) + 1;

	/*now check the balance_factor*/
	return adjust_balanace_factor_avltree(parent, root);
}


AVL_TREE_PTR SearchAvlTree(AVL_TREE_PTR start, AVL_TREE_PTR search_node)
{
	return (AVL_TREE_PTR)SearchBinaryTree((BINARY_TREE_PTR)start, (BINARY_TREE_PTR)search_node);
}
