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

FILE * gfp=NULL;

COMPARISION_RESULT compare_number(struct binary_tree * node1, struct binary_tree * node2);
BT_TEST_PTR InitBT_TestNode(BT_TEST_PTR node, int data);
void print_tree_graph(AVL_TREE_PTR node, char * name);
void print_tree(AVL_TREE_PTR avl_node);
int * init_numbers(int * total_numbers, int ** del_numbers_ptr, int * total_del_numbers, int allow_duplicates);
int parse_arguments(int argc, char * argv[]);
static int EnumerateAvlTreeCallback(AVL_TREE_PTR node, void * arg);

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
	
	print_tree_graph( root_ptr, "initial.dot");
	
	printf("\nEnumerating : ");
	EnumerateAvlTree(root_ptr, EnumerateAvlTreeCallback, NULL);
	printf("\n");
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
		if ( duplicate_node && del_numbers[del_number_index] == duplicate_node->data )
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
				print_tree_graph( root_ptr, "error.dot");
				return 1;
			}
			if ( verbose_level > 1 ) printf("Sucess\n");
		}
		else
		{
			printf("Node (%d)(%d) not found while deleting \n", del_node.data, del_number_index);
			print_tree_graph( root_ptr, "error.dot");
			return 1;
		}
	}
	if ( root_ptr )
	{
		print_tree_graph( root_ptr, "final.dot");
	}
	if ( verbose_level > 0 )
		printf("\nNormal exit\n");
	return 0;
}

BT_TEST_PTR InitBT_TestNode(BT_TEST_PTR node, int data)
{
	InitAvlTreeNode(&node->t, ALLOW_DUPLICATE);
#if ALLOW_DUPLICATE==1
	InitList( &node->tree_sibling);
#endif
	node->data = data;
	return node;
}

static int EnumerateAvlTreeCallback(AVL_TREE_PTR avl_node, void *arg)
{
	printf("%02d, ", STRUCT_ADDRESS_FROM_MEMBER(avl_node, BT_TEST, t)->data );
	return 0;
}

void print_tree_graph(AVL_TREE_PTR node, char * name)
{
	gfp = fopen( name, "w" );
	if ( gfp == NULL )
	{
		perror("fopen - ");
		return;
	}
	fprintf( gfp, "digraph avl_tree{\n");
	print_tree(node);
	fprintf( gfp, "}");
	fclose( gfp );
}

void print_tree(AVL_TREE_PTR avl_node)
{
	int data = STRUCT_ADDRESS_FROM_MEMBER(avl_node, BT_TEST, t)->data;
	if (! IS_AVL_TREE_RIGHT_LIST_END(avl_node) )
	{
		int right_data = STRUCT_ADDRESS_FROM_MEMBER(AVL_TREE_RIGHT_NODE(avl_node), BT_TEST, t)->data;
		fprintf( gfp, "%d->%d;\n", data, right_data );
		print_tree( AVL_TREE_RIGHT_NODE(avl_node) );
	}
	
	if (! IS_AVL_TREE_LEFT_LIST_END(avl_node ) )
	{
		int left_data = STRUCT_ADDRESS_FROM_MEMBER(AVL_TREE_LEFT_NODE(avl_node), BT_TEST, t)->data;
		fprintf( gfp, "%d->%d;\n", data, left_data );
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
	
	if ( n1 > n2 )
		return GREATER_THAN;
	else if ( n1 < n2 )
		return LESS_THAN;
	else 
		return EQUAL;
}
