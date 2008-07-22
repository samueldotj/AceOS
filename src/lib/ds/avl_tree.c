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

#include <ds/avl_tree.h>

#define GET_AVL_TREE_HEIGHTS( node, left_height, right_height )					\
	left_height = right_height = -1;											\
	if ( !IS_END_OF_LEFT_LIST( &(node)->bintree ) )								\
		left_height = AVL_TREE_LEFT_NODE( (node) )->height;						\
	if ( !IS_END_OF_RIGHT_LIST( &(node)->bintree ) ) 							\
		right_height = AVL_TREE_RIGHT_NODE( (node) )->height;	

#define RECALCULATE_HEIGHT( node ) (node)->height = RecalculateAvlTreeHeight( node )


/*caculate binary root ptr from AVL root pointer*/
#define CALCULATE_ROOT_PTR_AVL_TO_BT(avl_root_ptr, bt_root_ptr)												\
	if ( *avl_root_ptr )																					\
		*bt_root_ptr = (BINARY_TREE_PTR) ( ((UINT32)*avl_root_ptr) + OFFSET_OF_MEMBER(AVL_TREE, bintree) );	\
	else																									\
		*bt_root_ptr = NULL;

/*caculate AVL root pointer from binary tree root pointer*/
#define CALCULATE_ROOT_PTR_BT_TO_AVL(avl_root_ptr, bt_root_ptr)												\
	if ( *bt_root_ptr )																						\
		*avl_root_ptr = STRUCT_ADDRESS_FROM_MEMBER(*bt_root_ptr, AVL_TREE, bintree);						\
	else																									\
		*avl_root_ptr = NULL;

/*declarations go here*/
static int BalanceAvlTree(AVL_TREE_PTR start_node, AVL_TREE_PTR *root_ptr);
static int GetAvlTreeBalanceFactor(AVL_TREE_PTR node);
static int RecalculateAvlTreeHeight(AVL_TREE_PTR node);

static void RightRotateAvlTree(AVL_TREE_PTR node, AVL_TREE_PTR *root_ptr);
static void LeftRotateAvlTree(AVL_TREE_PTR  node, AVL_TREE_PTR *root_ptr);


/*! Initializes the avl tree structure.
 */
AVL_TREE_PTR InitAvlTreeNode(AVL_TREE_PTR node)
{
	InitBinaryTreeNode(&node->bintree);
	node->height=0;
	
	return node;
}

/*! Searches the AVL tree and returns the identified node
		Searching AVL tree is same as searching a binary tree
*/
AVL_TREE_PTR SearchAvlTree(AVL_TREE_PTR start, AVL_TREE_PTR search_node, void * fnCompare)
{
	BINARY_TREE_PTR bt = SearchBinaryTree(&start->bintree, &search_node->bintree, fnCompare);
	if ( bt )
		return STRUCT_ADDRESS_FROM_MEMBER(bt, AVL_TREE, bintree);
	else
		return NULL;
}

/*! gets the avl tree node's parent
*/
AVL_TREE_PTR GetAvlTreeNodeParent(AVL_TREE_PTR node)
{
	BINARY_TREE_PTR parent= GetTreeNodeParent(&(node)->bintree, NULL);
	if ( parent )
		return STRUCT_ADDRESS_FROM_MEMBER( parent , AVL_TREE, bintree ); 
	else
		return NULL;
}
/*! Removes the given node from the AVL tree.
	Algo:
		1) Remove the node using binary tree function
		2) Balance the AVL Tree
*/
int RemoveNodeFromAvlTree(AVL_TREE_PTR *avl_root_ptr, AVL_TREE_PTR node, int duplicates_allowed,  void * fnCompare)
{
	AVL_TREE_PTR parent=NULL;
	BINARY_TREE_PTR bt_parent_ptr, bt_root_ptr, sibling_ptr=NULL;
	int result, old_height;
	
	old_height = node->height;
	
	/*calculate binary tree root pointer*/
	CALCULATE_ROOT_PTR_AVL_TO_BT(avl_root_ptr, &bt_root_ptr);
	CALCULATE_ROOT_PTR_AVL_TO_BT(&parent, &bt_parent_ptr);
		
	result = RemoveNodeFromBinaryTree(&node->bintree, &bt_parent_ptr, &bt_root_ptr, duplicates_allowed, &sibling_ptr, fnCompare);
	
	/*adjust for the avl tree root pointer*/
	CALCULATE_ROOT_PTR_BT_TO_AVL(avl_root_ptr, &bt_root_ptr);		
	CALCULATE_ROOT_PTR_BT_TO_AVL(&parent, &bt_parent_ptr);

	/*return if we just removed a duplicate*/
	if ( duplicates_allowed && result == 1 )
	{	
		AVL_TREE_PTR new_node;
		
		assert( sibling_ptr != NULL );
		new_node = STRUCT_ADDRESS_FROM_MEMBER( sibling_ptr, AVL_TREE, bintree);
		new_node->height = old_height;
		
		return 0;
	}
	
	if (!parent) /* parent is null means I just removed root! */
		return 0;
	
	/*update the height of this node*/
	RECALCULATE_HEIGHT(parent);
	
	/*now check the balance_factor*/
	return BalanceAvlTree(parent, avl_root_ptr);
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
int InsertNodeIntoAvlTree(AVL_TREE_PTR * avl_root_ptr, AVL_TREE_PTR new_node, int duplicates_allowed, void * fnCompare)
{
	AVL_TREE_PTR parent, grand_parent;
	BINARY_TREE_PTR bt_root_ptr;
	int result;
	
	/*calculate binary tree root pointer*/
	CALCULATE_ROOT_PTR_AVL_TO_BT(avl_root_ptr, &bt_root_ptr);
		
	new_node->height=0;
	result = InsertNodeIntoBinaryTree( &bt_root_ptr, &new_node->bintree, duplicates_allowed, fnCompare);
	
	CALCULATE_ROOT_PTR_BT_TO_AVL(avl_root_ptr, &bt_root_ptr);
	
	if ( duplicates_allowed && result == 1 )
		return 1;/*duplicates allowed and it is added*/
	if ( result == -1 ) 
		return -1;/*duplicates not allowed*/
		
	if (new_node == *avl_root_ptr) {
		return 0;/*success*/
	}
	
	/* Balance the entire tree. Parent will always be balanced; So get grand-parent and start balancing from there. */
	parent = AVL_TREE_PARENT_NODE(new_node);
	if (parent == *avl_root_ptr)
	{
		/* parent is the root, so tree is balanced, only update the height*/
		RECALCULATE_HEIGHT(parent);
		return 0;
	}
	RECALCULATE_HEIGHT(parent);
	grand_parent = AVL_TREE_PARENT_NODE(parent);
	if (grand_parent == parent) 
	{
		/*No grand parent, so tree is balanced*/
		return 0;
	}
	return BalanceAvlTree(grand_parent, avl_root_ptr);
}

/*! Recalculates the AVL Tree height and returns the value
*/
static int RecalculateAvlTreeHeight(AVL_TREE_PTR node)
{
	int left_height, right_height;
	GET_AVL_TREE_HEIGHTS( node, left_height, right_height );
	return ( left_height>right_height?left_height:right_height ) + 1;
}

/*! Returns the balance factor for the given AVL tree node
*/
static int GetAvlTreeBalanceFactor(AVL_TREE_PTR node)
{
	int left_height, right_height;
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
static int BalanceAvlTree(AVL_TREE_PTR start_node, AVL_TREE_PTR *root_ptr)
{
	int balance_factor;
	AVL_TREE_PTR parent;

	assert(start_node != NULL);

	RECALCULATE_HEIGHT(start_node);
	balance_factor = GetAvlTreeBalanceFactor(start_node);
	assert( balance_factor >= -2 );
	assert( balance_factor <= 2 );

	switch(balance_factor) 
	{
		case 0:
		case 1:
		case -1: /*tree is balanced*/
			break;
		case 2: /*heavy on right*/
			LeftRotateAvlTree(start_node, root_ptr);
			break;
		case -2: /*heavy on left*/
			RightRotateAvlTree(start_node, root_ptr);
			break;
	}

	if (start_node == *root_ptr) {
		return 0;
	}

	parent = AVL_TREE_PARENT_NODE(start_node);
	/* start_node is now a child and hence it's parent is also balanced continue recursion till root */
	BalanceAvlTree(parent, root_ptr);
	return 0;
}

/*! Single or double right rotates the AVL tree on the node to balance
*/
void RightRotateAvlTree(AVL_TREE_PTR node, AVL_TREE_PTR *avl_root_ptr)
{
	AVL_TREE_PTR child = AVL_TREE_LEFT_NODE(node);
	int balance_factor = GetAvlTreeBalanceFactor(child);
	BINARY_TREE_PTR bt_root_ptr;
	
	/*only -1,0 or 1 balance is expected*/
	assert( balance_factor<=1 && balance_factor>=-1 );
	
	/*check for double rotate*/
	if ( balance_factor == 1 ) {
		/*calculate binary tree root pointer*/
		CALCULATE_ROOT_PTR_AVL_TO_BT(avl_root_ptr, &bt_root_ptr);
		RotateLeft( &child->bintree, &bt_root_ptr);
		CALCULATE_ROOT_PTR_BT_TO_AVL(avl_root_ptr, &bt_root_ptr);
	}

	/*single right rotate*/
	CALCULATE_ROOT_PTR_AVL_TO_BT(avl_root_ptr, &bt_root_ptr);
	RotateRight( &node->bintree, &bt_root_ptr);
	CALCULATE_ROOT_PTR_BT_TO_AVL(avl_root_ptr, &bt_root_ptr);

	/*calculate the new heights*/
	RECALCULATE_HEIGHT(node);
	RECALCULATE_HEIGHT(child);
}

/*! Single or double left rotates the AVL tree on the node to balance
*/
void LeftRotateAvlTree(AVL_TREE_PTR node, AVL_TREE_PTR *avl_root_ptr)
{
	AVL_TREE_PTR child = AVL_TREE_RIGHT_NODE(node);
	int balance_factor = GetAvlTreeBalanceFactor(child);
	BINARY_TREE_PTR bt_root_ptr;
	
	/*only -1,0 or 1 balance is expected*/
	assert( balance_factor<=1 && balance_factor>=-1 );
	
	/*check for double rotate*/
	if ( balance_factor == -1 ) {
		CALCULATE_ROOT_PTR_AVL_TO_BT(avl_root_ptr, &bt_root_ptr);
		RotateRight( &child->bintree, &bt_root_ptr);
		CALCULATE_ROOT_PTR_BT_TO_AVL(avl_root_ptr, &bt_root_ptr);
	}
	
	/*single rotate*/
	CALCULATE_ROOT_PTR_AVL_TO_BT(avl_root_ptr, &bt_root_ptr);
	RotateLeft( &node->bintree, &bt_root_ptr);
	CALCULATE_ROOT_PTR_BT_TO_AVL(avl_root_ptr, &bt_root_ptr);
	
	/*calculate the new heights*/
	RECALCULATE_HEIGHT(node);
	RECALCULATE_HEIGHT(child);
}
