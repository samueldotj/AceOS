#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <malloc.h>
#include <ds/binary_tree.h>

/*define the following to allow duplicate keys in the tree structure*/
#define ALLOW_DUPLICATE		1

struct bt_test
{
	BINARY_TREE t;
#if ALLOW_DUPLICATE==1
	LIST tree_sibling;
#endif

	int data;
};
typedef struct bt_test BT_TEST, * BT_TEST_PTR;

FILE * gfp=NULL;

COMPARISION_RESULT compare_number(struct binary_tree * node1, struct binary_tree * node2);
BT_TEST_PTR InitBT_TestNode(BT_TEST_PTR node, int data);
int EnumerateBinaryTreeCallback(BINARY_TREE_PTR node, void * arg);
void print_tree_graph(BINARY_TREE_PTR node, char * name);
void print_tree(BINARY_TREE_PTR node);
int * init_numbers(int * total_numbers, int ** del_numbers_ptr, int * total_del_numbers, int allow_duplicates);
int parse_arguments(int argc, char * argv[]);

int EnumerateBinaryTreeCallback(BINARY_TREE_PTR node, void * arg);

extern int verbose_level;
int main(int argc, char* argv[])
{
	int i, * numbers, total_numbers, * del_numbers, del_number_index;
	BINARY_TREE_PTR root_ptr = NULL;
	BT_TEST_PTR duplicate_node = NULL;
		
	if ( parse_arguments(argc, argv) )
		return 1;
	numbers = init_numbers(&total_numbers, &del_numbers, &del_number_index, ALLOW_DUPLICATE);
	
	/*insert into list*/
	if ( verbose_level > 0 ) printf("Inserting %d numbers between 0 to 100\n", total_numbers);
	for(i=0;i<total_numbers;i++)
	{
		BT_TEST_PTR new_node;
		int result;
		
		if ( (new_node = (BT_TEST_PTR ) malloc(sizeof(BT_TEST))) == NULL )
		{
			perror("malloc");
			return 1;
		}
		
		InitBT_TestNode(new_node, numbers[i]);
		if ( verbose_level > 1 ) printf("Adding node %p (%d) : ", &new_node->t, numbers[i] );
		
		result = InsertNodeIntoBinaryTree(&root_ptr, &new_node->t, ALLOW_DUPLICATE, compare_number );
		if ( ALLOW_DUPLICATE &&  result == 1 )
			duplicate_node = new_node;
		
		if ( !ALLOW_DUPLICATE &&  result != 0 )
		{
			printf("failure\n");
			return 1;
		}
		else
			if ( verbose_level > 1 ) printf("success\n");
		
	}
	printf("\nEnumerating : ");
	EnumerateBinaryTree(root_ptr, EnumerateBinaryTreeCallback, NULL);
	printf("\n");
	
	print_tree_graph(root_ptr, "initial.dot");
	
	if ( verbose_level > 0 )
		printf("\nDeleting %d numbers from the tree\n", del_number_index);
	
	/*duplicate node delete*/
	if ( duplicate_node )
	{
		printf("Deleting duplicate node %d\n", duplicate_node->data);
		RemoveNodeFromBinaryTree(&duplicate_node->t, NULL, &root_ptr, ALLOW_DUPLICATE, NULL, compare_number);
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
		
		if ( verbose_level > 1 ) printf("Searching %d (%d): ", del_node.data, del_number_index);
		BINARY_TREE_PTR del = SearchBinaryTree(root_ptr, &del_node.t, compare_number);
		if ( del )
		{
			if ( verbose_level > 1 ) printf("found. Deleting it : ");
			RemoveNodeFromBinaryTree(del, NULL, &root_ptr, ALLOW_DUPLICATE, NULL, compare_number);
			if ( root_ptr == NULL )
			{
				printf("Root node deleted\n");
				break;
			}
			if ( !ALLOW_DUPLICATE && SearchBinaryTree(root_ptr, &del_node.t, compare_number) )
			{
				printf("Deleted node still exists(%d)\n", del_node.data);
				print_tree_graph(root_ptr, "error.dot");
				return 1;
			}
			else
				if ( verbose_level > 1 ) printf("success\n");
		}
		else
		{
			printf("Node (%d) index %d not found while deleting \n", del_node.data, del_number_index);
			print_tree_graph(root_ptr, "error.dot");
			return 1;
		}
	}
	if ( root_ptr )
	{
		print_tree_graph(root_ptr, "final.dot");
	}
	if ( verbose_level > 1 ) 
		printf("\nNormal exit\n");
	return 0;
}

BT_TEST_PTR InitBT_TestNode(BT_TEST_PTR node, int data)
{
	InitBinaryTreeNode(&node->t, ALLOW_DUPLICATE);
#if ALLOW_DUPLICATE==1
	InitList( &node->tree_sibling );
#endif

	node->data = data;
	return node;
}

COMPARISION_RESULT compare_number(struct binary_tree * node1, struct binary_tree * node2)
{
	int n1, n2;
	
	assert( node1 != NULL );
	assert( node2 != NULL );
	
	n1 = STRUCT_ADDRESS_FROM_MEMBER(node1, BT_TEST, t)->data;
	n2 = STRUCT_ADDRESS_FROM_MEMBER(node2, BT_TEST, t)->data;
	
	if ( n1 > n2 )
		return GREATER_THAN;
	else if ( n1 < n2 )
		return LESS_THAN;
	else 
		return EQUAL;
}

int EnumerateBinaryTreeCallback(BINARY_TREE_PTR node, void * arg)
{
	printf("%02d, ", STRUCT_ADDRESS_FROM_MEMBER(node, BT_TEST, t)->data );
	return 0;
}

void print_tree_graph(BINARY_TREE_PTR node, char * name)
{
	gfp = fopen( name, "w" );
	if ( gfp == NULL )
	{
		perror("fopen - ");
		return;
	}
	fprintf( gfp, "digraph bin_tree{\n");
	print_tree(node);
	fprintf( gfp, "}");
	fclose( gfp );
}

void print_tree(BINARY_TREE_PTR node)
{
	int data = STRUCT_ADDRESS_FROM_MEMBER(node, BT_TEST, t)->data;
	if (! IS_TREE_LIST_END(&node->right) )
	{
		int right_data = STRUCT_ADDRESS_FROM_MEMBER(TREE_RIGHT_NODE(node), BT_TEST, t)->data;
		fprintf( gfp, "%d->%d;\n", data, right_data );
		print_tree( TREE_RIGHT_NODE(node) );
	}
	
	if (! IS_TREE_LIST_END(&node->left ) )
	{
		int left_data = STRUCT_ADDRESS_FROM_MEMBER(TREE_LEFT_NODE(node), BT_TEST, t)->data;
		fprintf( gfp, "%d->%d;\n", data, left_data );
		print_tree( TREE_LEFT_NODE(node) );
	}
}
