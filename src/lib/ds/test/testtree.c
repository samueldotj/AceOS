#include <binary_tree.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct bt_test
{
	BINARY_TREE t;
	int data;
};
typedef struct bt_test BT_TEST, * BT_TEST_PTR;

COMPARISION_RESULT compare_number(struct binary_tree * node1, struct binary_tree * node2);
BT_TEST_PTR InitBT_TestNode(BT_TEST_PTR node, int data);

void print_tree(BINARY_TREE_PTR node);
int * init_numbers(int * total_numbers, int ** del_numbers_ptr, int * total_del_numbers);
int parse_arguments(int argc, char * argv[]);

int main(int argc, char* argv[])
{
	BT_TEST first_element;
	BT_TEST_PTR search_node=NULL;
	int i, * numbers, total_numbers, * del_numbers, del_number_index;
	BINARY_TREE_PTR root_ptr = &first_element.t;
		
	if ( parse_arguments(argc, argv) )
		return;
	numbers = init_numbers(&total_numbers, &del_numbers, &del_number_index);
		
	InitBT_TestNode( &first_element, 150);
	
	/*insert into list*/
	printf("Inserting 20 numbers between 0 to 100\n");
	for(i=0;i<total_numbers;i++)
	{
		BT_TEST_PTR new_node;
		
		if ( (new_node = (BT_TEST_PTR ) malloc(sizeof(BT_TEST))) == NULL )
		{
			perror("malloc");
			return -1;
		}
		
		InitBT_TestNode(new_node, numbers[i]);
		printf("Adding node %p (%d) : ", &new_node->t, numbers[i] );
		if ( InsertNodeIntoBinaryTree(root_ptr, &new_node->t ) != 0 )
		{
			printf("failure\n");
			return;
		}
		else
			printf("success\n");
		
	}
	print_tree(root_ptr);
	printf("\n");
	
	/*deletion test*/
	del_number_index--;
	for(;del_number_index>=0;del_number_index--)
	{
		int result;
		BT_TEST del_node;
		InitBT_TestNode(&del_node, del_numbers[del_number_index]);
		
		printf("Searching %d : ", del_node.data);
		BINARY_TREE_PTR del = SearchBinaryTree(root_ptr, &del_node.t);
		if ( del )
		{
			printf("found. Deleting it : ");
			RemoveNodeFromBinaryTree(del, NULL, &root_ptr);
			if ( SearchBinaryTree(root_ptr, &del_node.t) )
			{
				printf("node still exists\n");
				print_tree(root_ptr);
				return 1;
			}
			else
				printf("success\n");
		}
		else
		{
			printf("***********Not found**************\n");
		}
	}
	printf("-------------------FINAL TREE---------------------------------------------\n");
	print_tree(root_ptr);
	printf("\n--------------------------------------------------------------------------\n");
	
	printf("\nNormal exit\n");
	return 0;
}

BT_TEST_PTR InitBT_TestNode(BT_TEST_PTR node, int data)
{
	InitBinaryTreeNode(&node->t, compare_number);

	node->data = data;
	return node;
}

COMPARISION_RESULT compare_number(struct binary_tree * node1, struct binary_tree * node2)
{
	int n1, n2;
	
	assert( node1 != NULL );
	assert( node2 != NULL );
	
	n1 = STRUCT_FROM_MEMBER(BT_TEST_PTR, t, node1)->data;
	n2 = STRUCT_FROM_MEMBER(BT_TEST_PTR, t, node2)->data;
	
	if ( n1 < n2 )
		return GREATHER_THAN;
	else if ( n1 > n2 )
		return LESS_THAN;
	else 
		return EQUAL;
}

void print_tree(BINARY_TREE_PTR node)
{
	static int i=0, level=0;
	int len = printf("%02d", STRUCT_FROM_MEMBER(BT_TEST_PTR, t, node)->data )+1;
	if (! IS_TREE_LIST_END(&node->right) )
	{
		level++;
		printf("-");
		print_tree( TREE_RIGHT_NODE(node) );
		level--;
	}
	
	if (! IS_TREE_LIST_END(&node->left ) )
	{
		int j=0;
		printf("\n");
		
		for (j=0 ;j<(level*len);j++ ) printf(" ");
		printf("|\n");
		for (j=0 ;j<(level*len);j++ ) printf(" ");
		print_tree( TREE_LEFT_NODE(node) );
	}
}
