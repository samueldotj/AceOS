/*!
	\file		binary_tree.c
	\author		Samuel
	\version 	1.0
	\date	
  			Created: 04-Feb-2008 18:24
  			Last modified: 04-Feb-2008 18:24
	\brief	Generic binary tree implementation
	
*/

#include "binary_tree.h"
#include <assert.h>

#define MARK_TREELIST_END(list)		(list)->next = (LIST_PTR) (((unsigned long)(list)->next ) | 1);

#define MARK_TREE_NODE_AS_END(node)													\
	(node)->left.next 	= (LIST_PTR) (((unsigned long)(node)->left.next ) | 1);		\
	(node)->right.next 	= (LIST_PTR) (((unsigned long)(node)->right.next ) | 1);	\

#define REMOVE_END_MARK(list)		(list)->next = (LIST_PTR) (((unsigned long)(list)->next ) & ~1);

#define COMPARE_NODE(root, search_node, result_node, result) 	\
	result = root->fnCompareKey(root, search_node);				\
	switch( result )											\
	{															\
		case LESS_THAN:											\
			result_node = TREE_LEFT_NODE(root);					\
			break;												\
		case GREATHER_THAN:										\
			result_node = TREE_RIGHT_NODE(root);				\
			break;												\
		default:												\
			result_node = NULL;									\
			break;												\
	}

/*! Returns parent node of given node and also returns on which list(left or right) the node is attached with the parent.
	If the node has no parent(root), NULL is returned
*/
BINARY_TREE_PTR GetTreeNodeParent(BINARY_TREE_PTR node, TREE_LIST_TYPE * list_type)
{
	BINARY_TREE_PTR left_parent, right_parent;
	left_parent = TREE_LEFT_PARENT( node );
	right_parent = TREE_RIGHT_PARENT( node );
	
	if ( !IS_END_OF_LEFT_LIST(left_parent) && left_parent!=node &&  TREE_LEFT_NODE(left_parent)==node )
	{
		*list_type = LEFT_TREE_LIST;
		return left_parent;
	}
	
	if ( !IS_END_OF_RIGHT_LIST(right_parent) && right_parent!=node && TREE_RIGHT_NODE(right_parent)==node)
	{
		*list_type = RIGHT_TREE_LIST;
		return right_parent;
	}

	/*root doesnt have any parent*/	
	*list_type = NO_LIST;
	return NULL;
}

/*! Initializes the binary tree structure.

*/
BINARY_TREE_PTR InitBinaryTreeNode(BINARY_TREE_PTR node, COMPARISION_RESULT (*compare_function_ptr)(struct binary_tree * , struct binary_tree *))
{
	InitList( &node->left );
	InitList( &node->right );
	
	node->fnCompareKey = compare_function_ptr;
	
	MARK_TREE_NODE_AS_END(node);

	return node;
}

	
/*! Searches the binary tree and returns the matching node if found.
	If not found NULL is returned.
*/
BINARY_TREE_PTR SearchBinaryTree(BINARY_TREE_PTR root, BINARY_TREE_PTR search_node)
{
	COMPARISION_RESULT result;
	BINARY_TREE_PTR next_node;
	
	assert( root != NULL );
	assert( search_node != NULL );
	
	while(1)
	{
		COMPARE_NODE( root, search_node, next_node, result );
		/*node found*/
		if ( result == EQUAL )
			return root;

		/*end of list - node not found*/
		if ( ( result == LESS_THAN     && IS_TREE_LIST_END(&root->left) ) || 
			 ( result == GREATHER_THAN && IS_TREE_LIST_END(&root->right) ) )
			return NULL;
		
		
		root = next_node;
	}
	
	/*to satisfy compiler*/
	return NULL;
}

/*! Inserts an already created node into the binary tree.
	Return values:
		0    SUCCESS
		-1   FAIL (Duplicate value, not inserted)
	Theory:
		All binary tree inserts happens only in the leaf nodes
	Algorithm:
		1) Traverse through the node from root
		2) Reset the root variable if the code branches to left on "right list" and right on "left list"
*/
int InsertNodeIntoBinaryTree(BINARY_TREE_PTR root, BINARY_TREE_PTR new_node)
{
	COMPARISION_RESULT result;
	BINARY_TREE_PTR next_node;
	
	assert( root != NULL );
	assert( new_node != NULL );
	
	while(1)
	{
		LIST_PTR parent_node_list;
		
		COMPARE_NODE( root, new_node, next_node, result);
		if ( result == EQUAL )
			return -1;/*duplicate*/
		
		if ( result == LESS_THAN )
			parent_node_list = &root->left;
		else if ( result == GREATHER_THAN )
			parent_node_list = &root->right;
			
		//end of list - inser the node here
		if ( IS_TREE_LIST_END(parent_node_list) )
		{
			//Initialize the List datastructure
			InitList(&new_node->left);
			InitList(&new_node->right);
	
			parent_node_list->next = (LIST_PTR) ( (unsigned long)parent_node_list->next & ~1 );
			AddToList( parent_node_list, (result==LESS_THAN) ? &new_node->left : &new_node->right );
			
			MARK_TREE_NODE_AS_END(new_node);
			return 0;
		}
		root = next_node;
	}
}
/*! Removes the given node from the Tree.
	Return values:
		0 - Sucess
		
	Cases:
		case 1(leaf - no left or right node)
		case 2(only left node)
		case 3(only right node)
		case 4(both left and right nodes present)
*/
int RemoveNodeFromBinaryTree(BINARY_TREE_PTR node)
{
	TREE_LIST_TYPE in_list_type;
	BINARY_TREE_PTR left_node, right_node;
	BINARY_TREE_PTR right_most_node, parent_node;
	int is_left_end, is_right_end;
	
	assert(node!=NULL);
	
	left_node = TREE_LEFT_NODE(node);
	right_node = TREE_RIGHT_NODE(node);
	parent_node = GetTreeNodeParent(node, &in_list_type);
	
	is_left_end = IS_END_OF_LEFT_LIST(node);
	is_right_end = IS_END_OF_RIGHT_LIST(node);
	
	//case 1, 2 or 3
	if ( is_left_end || is_right_end )
	{
		if ( in_list_type == LEFT_TREE_LIST )
		{
			//case 1 or 2
			UnlinkTreeList( &node->left );
			//special case left is empty but right node exists
			if( is_left_end && !is_right_end )
			{
				UnlinkTreeList( &node->right );
				LinkTwoTreeLists( &parent_node->left, &right_node->left);
			}
		}
		if ( in_list_type == RIGHT_TREE_LIST )
		{
			//case 1 or 3
			UnlinkTreeList( &node->right );
			//special case right is empty but left node exists
			if( is_right_end && !is_left_end )
			{	
				UnlinkTreeList( &node->left );
				LinkTwoTreeLists( &parent_node->right, &left_node->right);
			}
		}
		return 0;
		
	}
	
	/*case 4(both left and right nodes present)
	right_most_node = "Right most node" of the "left node" of the del_node
		1) Remove the right_most_node
		2) Replace current node with right_most_node
	*/
	//find right_most_node
	right_most_node = TREE_RIGHT_PARENT( left_node );
	
	//remove right_most_node
	RemoveNodeFromBinaryTree(right_most_node);
	//reinitialize the pointer
	InitBinaryTreeNode(right_most_node, right_most_node->fnCompareKey);
	
	//Replace current node with right_most_node
	ReplaceTreeListNode(&node->left, &right_most_node->left);
	ReplaceTreeListNode(&node->right, &right_most_node->right);

	return 0;
}

/*! Unlinks the given tree "list" node from the tree "list".
	This function takes care of removing END marks and putting it back.
*/
static void UnlinkTreeList(LIST_PTR list_node)
{
	LIST_PTR prev_node = NULL;
	
	int is_tail = IS_TREE_LIST_END( list_node );
	int is_head = IS_TREE_LIST_END( list_node->prev );
	
	prev_node = list_node->prev;
	
	/*Remove the END mark from the pointers before using it*/
	if ( is_tail )
		REMOVE_END_MARK( list_node );
	if ( is_head )
		REMOVE_END_MARK( list_node->prev );
	
	/*unlink the node*/
	RemoveFromList( list_node );
	
	/*put the END mark if required*/
	if ( is_tail )
		MARK_TREELIST_END( prev_node );
	if ( is_head )
		MARK_TREELIST_END( prev_node );
	
}
/*! Joins two tree lists.
	This function takes care of removing END marks and putting it back.
*/
static void LinkTwoTreeLists( LIST_PTR list1head, LIST_PTR list2head )
{
	LIST_PTR tail = list2head->prev;
	
	REMOVE_END_MARK(list1head );
	REMOVE_END_MARK(tail );
	LinkLists(list1head, list2head);
	MARK_TREELIST_END(tail);
}
/*!	Links the new tree node list with the prev and next of old tree node list
	This function takes care of removing END marks and putting it back.
*/
static void ReplaceTreeListNode(LIST_PTR old_node, LIST_PTR new_node)
{
	LIST_PTR prev_node, next_node;
	int is_tail, is_head;
	
	is_tail = IS_TREE_LIST_END( old_node );
	is_head = IS_TREE_LIST_END( old_node->prev );
	if ( is_tail )
		REMOVE_END_MARK( old_node );
	if ( is_head )
		REMOVE_END_MARK( old_node->prev );
	
	prev_node = old_node->prev;
	next_node = old_node->next;
	
	prev_node->next = new_node;
	new_node->prev = prev_node;
	
	next_node->prev = new_node;
	new_node->next = next_node;
	
	if( is_tail )
		MARK_TREELIST_END( new_node );
		
	if ( is_head )
		MARK_TREELIST_END( prev_node );
}
