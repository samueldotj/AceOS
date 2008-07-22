#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <malloc.h>
#include <ds/avl_tree.h>

/*define the following to allow duplicate keys in the tree structure*/
#define ALLOW_DUPLICATE		1

struct bt_test
{
	AVL_TREE t;
#if ALLOW_DUPLICATE==1
	LIST tree_sibling;
#endif
	int data;
};
typedef struct bt_test BT_TEST, * BT_TEST_PTR;

COMPARISION_RESULT compare_number(struct binary_tree * node1, struct binary_tree * node2);
BT_TEST_PTR InitBT_TestNode(BT_TEST_PTR node, int data);

void print_tree(AVL_TREE_PTR avl_node);
int * init_numbers(int * total_numbers, int ** del_numbers_ptr, int * total_del_numbers, int allow_duplicates);
int parse_arguments(int argc, char * argv[]);

extern int verbose_level;
int main(int argc, char * argv[])
{
	AVL_TREE_PTR root_ptr=NULL;
	BT_TEST_PTR duplicate_node = NULL;
	int i, * numbers, total_numbers, * del_numbers, del_number_index;
	
	
	if ( parse_arguments(argc, argv) )
		return 1;
	numbers = init_numbers(&total_numbers, &del_numbers, &del_number_index, ALLOW_DUPLICATE);
	
	/*insertion test*/
	if ( verbose_level > 0 ) printf("Inserting %d numbers between 0 to 100\n", total_numbers);
	for(i=0;i<total_numbers; i++)
	{
		BT_TEST_PTR new_node;
		int result;
		if ( (new_node = (BT_TEST_PTR ) malloc(sizeof(BT_TEST))) == NULL )
		{
			perror("malloc");
			return -1;
		}
		InitBT_TestNode(new_node, numbers[i]);
		
		if ( verbose_level > 1 ) printf("Adding node %p (%d) : ", &new_node->t, numbers[i] );
		
		result = InsertNodeIntoAvlTree( &root_ptr, &new_node->t, ALLOW_DUPLICATE, compare_number );
		if ( ALLOW_DUPLICATE && result == 1 )
		{
			duplicate_node = new_node;
		}
		if ( !ALLOW_DUPLICATE && result != 0 )
		{
			printf("InsertNodeIntoAvlTree() failed\n");
			return 1;
		}
		else
			if ( verbose_level > 1 ) printf("success\n");
		
	}
	if ( verbose_level > 1 )
	{
		printf("After insertion - Tree :\n");
		print_tree(root_ptr);
	}
	if ( verbose_level > 0 ) printf("\nDeleting %d numbers from the tree\n", del_number_index);
	
	/*duplicate node delete*/
	if ( duplicate_node )
	{
		printf("Deleting duplicate node %d\n", duplicate_node->data);
		RemoveNodeFromAvlTree( &root_ptr, &duplicate_node->t, ALLOW_DUPLICATE, compare_number);
	}
	
	/*deletion test*/
	del_number_index--;
	for(;del_number_index>=0;del_number_index--)
	{
		BT_TEST del_node;
		if ( del_numbers[del_number_index] == duplicate_node->data )
		{
			duplicate_node->data = -1;
			continue;
		}
		
		InitBT_TestNode(&del_node, del_numbers[del_number_index]);
		
		if ( verbose_level > 1 ) printf("Searching %d : ", del_node.data);
		AVL_TREE_PTR del = SearchAvlTree( root_ptr, &del_node.t, compare_number);
		if ( del )
		{
			if ( verbose_level > 1 ) printf("found. Deleting it : ");
			RemoveNodeFromAvlTree( &root_ptr, del, ALLOW_DUPLICATE, compare_number);
			if (!root_ptr)
			{
				printf("Last node deleted and hence no root\n");
				break;
			}
			if ( !ALLOW_DUPLICATE && SearchAvlTree( root_ptr, &del_node.t, compare_number) )
			{
				printf("Deleted node still exists(%d) index = %d\n", del_number_index, del_number_index);
				print_tree( root_ptr );
				return 1;
			}
			if ( verbose_level > 1 ) printf("Sucess\n");
		}
		else
		{
			printf("Node (%d)(%d) not found while deleting \n", del_node.data, del_number_index);
			print_tree( root_ptr );
			return 1;
		}
	}
	if ( root_ptr && verbose_level > 1 )
	{
		printf("-------------------FINAL TREE---------------------------------------------\n");
		print_tree( root_ptr );
		printf("\n--------------------------------------------------------------------------\n");
	}
	if ( verbose_level > 0 )
		printf("\nNormal exit\n");
	return 0;
}

BT_TEST_PTR InitBT_TestNode(BT_TEST_PTR node, int data)
{
	InitAvlTreeNode(&node->t);
#if ALLOW_DUPLICATE==1
	InitList( &node->tree_sibling);
#endif
	node->data = data;
	return node;
}


void print_tree(AVL_TREE_PTR avl_node)
{
	static int level=0;
	int len = printf("%02d(%d)", STRUCT_ADDRESS_FROM_MEMBER(avl_node, BT_TEST, t)->data, avl_node->height )+1;
	if (! IS_AVL_TREE_RIGHT_LIST_END(avl_node) )
	{
		level++;
		printf("-");
		print_tree( AVL_TREE_RIGHT_NODE(avl_node) );
		level--;
	}
	
	if (! IS_AVL_TREE_LEFT_LIST_END(avl_node ) )
	{
		int j=0;
		printf("\n");
		
		for (j=0 ;j<(level*len);j++ ) printf(" ");
		printf("|\n");
		for (j=0 ;j<(level*len);j++ ) printf(" ");
		
		print_tree( AVL_TREE_LEFT_NODE(avl_node) );
	}
	
}
COMPARISION_RESULT compare_number(struct binary_tree * node1, struct binary_tree * node2)
{
	int n1, n2;
	
	assert( node1 != NULL );
	assert( node2 != NULL );
	
	n1 = STRUCT_ADDRESS_FROM_MEMBER(STRUCT_ADDRESS_FROM_MEMBER(node1, AVL_TREE, bintree), BT_TEST, t)->data;
	n2 = STRUCT_ADDRESS_FROM_MEMBER(STRUCT_ADDRESS_FROM_MEMBER(node2, AVL_TREE, bintree), BT_TEST, t)->data;
	
	if ( n1 < n2 )
		return GREATER_THAN;
	else if ( n1 > n2 )
		return LESS_THAN;
	else 
		return EQUAL;
}
