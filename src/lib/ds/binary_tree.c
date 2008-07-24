/*!
	\file		binary_tree.c
	\author		Samuel
	\version 	1.0
	\date	
  			Created: 04-Feb-2008 18:24
  			Last modified: Fri May 02, 2008  11:21AM
	\brief	Generic binary tree implementation
	
*/

#include <ds/binary_tree.h>

#define MARK_TREELIST_END(list)		(list)->next = (LIST_PTR) (((unsigned long)(list)->next ) | 1);

#define MARK_TREE_NODE_AS_END(node)													\
	(node)->left.next 	= (LIST_PTR) (((unsigned long)(node)->left.next ) | 1);		\
	(node)->right.next 	= (LIST_PTR) (((unsigned long)(node)->right.next ) | 1);	\

#define REMOVE_END_MARK(list)		(list)->next = (LIST_PTR) (((unsigned long)(list)->next ) & ~1);

#define COMPARE_NODE(root, search_node, result_node, result, fnCompare) 	\
	result = fnCompare(root, search_node);									\
	switch( result )														\
	{																		\
		case LESS_THAN:														\
			result_node = TREE_LEFT_NODE(root);								\
			break;															\
		case GREATER_THAN:													\
			result_node = TREE_RIGHT_NODE(root);							\
			break;															\
		default:															\
			result_node = NULL;												\
			break;															\
	}

static void UnlinkTreeList(LIST_PTR list_node);
static void LinkTwoTreeLists( LIST_PTR list1head, LIST_PTR list2head );
static void ReplaceTreeListNode(LIST_PTR old_node, LIST_PTR new_node);
static void InsertNodeInTreeList( LIST_PTR list, LIST_PTR node );

	
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
		if ( list_type )
			*list_type = LEFT_TREE_LIST;
		return left_parent;
	}
	if ( !IS_END_OF_RIGHT_LIST(right_parent) && right_parent!=node && TREE_RIGHT_NODE(right_parent)==node)
	{
		if ( list_type )
			*list_type = RIGHT_TREE_LIST;
		return right_parent;
	}

	if (list_type)
		*list_type = NO_LIST;
	return NULL;
}

/*! Initializes the binary tree structure.

*/
BINARY_TREE_PTR InitBinaryTreeNode(BINARY_TREE_PTR node, int duplicates_allowed)
{
	InitList( &node->left );
	InitList( &node->right );
	
	MARK_TREE_NODE_AS_END(node);
	
	if ( duplicates_allowed ) 
		InitList( &node->sibling[0] );
	
	return node;
}

	
/*! Searches the binary tree and returns the matching node if found.
	If not found NULL is returned.
*/
BINARY_TREE_PTR SearchBinaryTree(BINARY_TREE_PTR root, BINARY_TREE_PTR search_node, COMPARISION_RESULT (*fnCompare)(BINARY_TREE_PTR, BINARY_TREE_PTR))
{
	COMPARISION_RESULT result;
	BINARY_TREE_PTR next_node;
	
	assert( root != NULL );
	assert( search_node != NULL );
	
	while(1)
	{
		COMPARE_NODE( root, search_node, next_node, result, fnCompare );
		/*node found*/
		if ( result == EQUAL )
			return root;

		/*end of list - node not found*/
		if ( ( result == LESS_THAN    && IS_TREE_LIST_END(&root->left) ) || 
			 ( result == GREATER_THAN && IS_TREE_LIST_END(&root->right) ) )
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
		1  Success Duplicate value added
	Theory:
		All binary tree inserts happens only in the leaf nodes
	Algorithm:
		1) Traverse through the node from root
		2) Reset the root variable if the code branches to left on "right list" and right on "left list"
*/
int InsertNodeIntoBinaryTree(BINARY_TREE_PTR * root_ptr, BINARY_TREE_PTR new_node, int duplicates_allowed, COMPARISION_RESULT (*fnCompare)(BINARY_TREE_PTR, BINARY_TREE_PTR))
{
	COMPARISION_RESULT result;
	BINARY_TREE_PTR root, next_node;
	
	assert( root_ptr != NULL );
	assert( new_node != NULL );
	
	InitBinaryTreeNode( new_node, duplicates_allowed );
	
	/*if this is the first node, just update the root_ptr and return*/
	if ( *root_ptr == NULL )
	{
		*root_ptr = new_node;
		return 0;
	}
	/*get the root element*/
	root = *root_ptr;
	
	/*walk through the tree and insert the node in correct place*/
	while(1)
	{
		LIST_PTR parent_node_list;
		
		COMPARE_NODE( root, new_node, next_node, result, fnCompare);
		if ( result == EQUAL )
		{
			if ( duplicates_allowed )
			{
				AddToList( &root->sibling[0], &new_node->sibling[0] );
				return 1;
			}
			else
				return -1;/*duplicate*/
		}
		
		if ( result == LESS_THAN )
			parent_node_list = &root->left;
		else if ( result == GREATER_THAN )
			parent_node_list = &root->right;
			
		//end of list - inser the node here
		if ( IS_TREE_LIST_END(parent_node_list) )
		{
			parent_node_list->next = (LIST_PTR) ( (unsigned long)parent_node_list->next & ~1 );
			AddToList( parent_node_list, (result==LESS_THAN) ? &new_node->left : &new_node->right );
			
			MARK_TREE_NODE_AS_END(new_node);
			return 0;
		}
		root = next_node;
	}
}
/*! Removes the given node from the Tree.
	Return values
		leaf_node - Points to the parent node of the deleted leaf node. 
					It is always a leaf(after deletion).
		root_ptr  - The new root pointer.
		
		Note: leaf_node and/or root_ptr can be null;
		
	Cases:
		case 1(leaf - no left or right node)
		case 2(only left node)
		case 3(only right node)
		case 4(both left and right nodes present)
*/
int RemoveNodeFromBinaryTree(BINARY_TREE_PTR node, BINARY_TREE_PTR * leaf_node, BINARY_TREE_PTR * root_ptr, 
				int duplicates_allowed, BINARY_TREE_PTR * sibling_ptr, COMPARISION_RESULT (*fnCompare)(BINARY_TREE_PTR, BINARY_TREE_PTR))
{
	TREE_LIST_TYPE in_list_type;
	BINARY_TREE_PTR left_node, right_node;
	BINARY_TREE_PTR right_most_node, parent_node;
	int is_left_end, is_right_end;
	
	assert(node!=NULL);
	
	/*if duplicates are allowed, check the sibling and replace with the current node*/
	if ( duplicates_allowed )
	{
		/*if sibling present*/
		if ( node->sibling[0].next != &node->sibling[0] )
		{
			BINARY_TREE_PTR new_node = STRUCT_ADDRESS_FROM_MEMBER( node->sibling[0].next, BINARY_TREE, sibling[0] );
			if ( TREE_LEFT_NODE( node ) != node )
				ReplaceTreeListNode( &node->left, &new_node->left );
			if ( TREE_RIGHT_NODE( node ) != node )
				ReplaceTreeListNode( &node->right, &new_node->right );
			
			RemoveFromList( &node->sibling[0] );

			/*update the sibling ptr*/
			if ( sibling_ptr )
				* sibling_ptr = new_node;
			
			/*change the root pointer if required*/
			if ( root_ptr && *root_ptr == node )
				*root_ptr = new_node;
			return 1;
		}
	}
		
	left_node = TREE_LEFT_NODE(node);
	right_node = TREE_RIGHT_NODE(node);
	parent_node = GetTreeNodeParent(node, &in_list_type);
	
	is_left_end = IS_END_OF_LEFT_LIST(node);
	is_right_end = IS_END_OF_RIGHT_LIST(node);

	if (is_left_end && is_right_end && parent_node == NULL)
	{ /* Special case when only 1 node exists in the tree, which is the root node */
		*root_ptr = NULL;
		if (leaf_node)
		{
			*leaf_node = NULL;
		}
		return 0; /*SUCCESS*/
	}

	//case 1, 2 or 3
	if ( is_left_end || is_right_end )
	{
		//leaf node
		if ( in_list_type == NO_LIST )
		{
			if ( !is_left_end )
				in_list_type = LEFT_TREE_LIST;
			else
				in_list_type = RIGHT_TREE_LIST;
		}

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
			//update the root pointer
			if ( root_ptr && parent_node==NULL )
				*root_ptr = left_node;
		}
		else if ( in_list_type == RIGHT_TREE_LIST )
		{
			//case 1 or 3
			UnlinkTreeList( &node->right );
			//special case right is empty but left node exists
			if( is_right_end && !is_left_end )
			{	
				UnlinkTreeList( &node->left );
				LinkTwoTreeLists( &parent_node->right, &left_node->right);
			}
			
			//update the root pointer
			if ( root_ptr && parent_node==NULL )
				*root_ptr = right_node;
		}
		
		/*if leaf_node passed, then point the new leaf_node.
		*/
		if ( leaf_node )
			*leaf_node = parent_node;
		
		return 0;/*success*/
	}
	
	/*case 4(both left and right nodes present)
	right_most_node = "Right most node" of the "left node" of the del_node
		1) Remove the right_most_node
		2) Replace current node with right_most_node
	*/
	//find right_most_node
	right_most_node = TREE_RIGHT_PARENT( left_node );

	//remove right_most_node
	RemoveNodeFromBinaryTree(right_most_node, leaf_node, root_ptr, 0, NULL, fnCompare);
	if (leaf_node && *leaf_node == node)
		*leaf_node = right_most_node;
	
	//Replace current node with right_most_node
	ReplaceTreeListNode(&node->left, &right_most_node->left);
	ReplaceTreeListNode(&node->right, &right_most_node->right);
	
	/*reassign the root pointer if it is changed.*/
	if ( root_ptr && parent_node==NULL )
		*root_ptr = right_most_node;
	
	return 0;
}

/*! Performs a right rotation, rooted at node
	and updates the root pointer if neccesary
	
	Algo for right rotation
		parent = node
		child = node->left
		1) Remove parent from the left list
		2) Link child's "right child" as parents left child
		3) Link parent as "right child" of child.
*/
void RotateRight(BINARY_TREE_PTR node, BINARY_TREE_PTR *root_ptr)
{
	BINARY_TREE_PTR parent, child;
	
	parent = node;
	child = TREE_LEFT_NODE(parent);
	
	//Remove parent from the left list
	UnlinkTreeList( &parent->left );
	
	//Link child's "right child" as parents left child
	if ( !IS_END_OF_RIGHT_LIST( child ) )
	{
		BINARY_TREE_PTR child_right_node = TREE_RIGHT_NODE(child);
		UnlinkTreeList( &child->right );
		LinkTwoTreeLists( &parent->left,  &child_right_node->left );
	}
	//Link parent as "right child" of child.
	InsertNodeInTreeList(&(parent->right), &(child->right));
	
	//updates the root pointer if neccesary
	if ( node == *root_ptr )
		*root_ptr = child;
}

/*! Performs a left rotation, rooted at node
	and updates the root pointer if neccesary
	
	Algo for left rotation
		parent = node
		child = node->right
		1) Remove parent from the right list
		2) Link child's "left child" as parents right child
		3) Link parent as "left child" of child.
*/
void RotateLeft(BINARY_TREE_PTR node, BINARY_TREE_PTR *root_ptr)
{
	BINARY_TREE_PTR parent, child;

	parent = node;
	child = TREE_RIGHT_NODE(parent);

	//Remove parent from the right list
	UnlinkTreeList( &parent->right );
	
	//Link child's "left child" as parents right child
	if ( !IS_END_OF_LEFT_LIST( child ) )
	{
		BINARY_TREE_PTR child_left_node = TREE_LEFT_NODE(child);
		UnlinkTreeList( &child->left );
		LinkTwoTreeLists( &parent->right,  &child_left_node->right );
	}

	//Link parent as "left child" of child.
	InsertNodeInTreeList(&(parent->left), &(child->left));
	
	//updates the root pointer if neccesary
	if ( node == *root_ptr )
		*root_ptr = child;
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
	MARK_TREELIST_END( list_node );
	
	/*put the END mark if required*/
	if ( is_tail )
		MARK_TREELIST_END( prev_node );
	if ( is_head )
		MARK_TREELIST_END( prev_node );
	
}

/*! Insert a node into the list 
	This function takes care of removing END marks and putting it back.
*/
static void InsertNodeInTreeList( LIST_PTR list, LIST_PTR node )
{
	int is_tail;
	LIST_PTR tail;

	is_tail =  IS_TREE_LIST_END(list->prev);
	
	tail = list->prev;
	REMOVE_END_MARK(node);
	REMOVE_END_MARK(tail);

	AddToList(tail, node);
	if (is_tail)
		MARK_TREELIST_END(tail);
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

	if (old_node != old_node->prev) /* More than 1 node exists, so modify the list */
	{
		prev_node = old_node->prev;
		next_node = old_node->next;

		prev_node->next = new_node;
		new_node->prev = prev_node;

		next_node->prev = new_node;
		new_node->next = next_node;

		old_node->prev = old_node->next = old_node;
	
		if ( is_head )
			MARK_TREELIST_END( prev_node );
	}

	MARK_TREELIST_END( old_node );

	if( is_tail )
		MARK_TREELIST_END( new_node );
}

