#include <binary_tree.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_TREE_NUMBERS 30
#define MAX_DEL_NUMBERS  10

int random_number_test=1;

struct bt_test
{
	BINARY_TREE t;
	int data;
};
typedef struct bt_test BT_TEST, * BT_TEST_PTR;

COMPARISION_RESULT compare_number(struct binary_tree * node1, struct binary_tree * node2);
BT_TEST_PTR InitBT_TestNode(BT_TEST_PTR node, int data);

void print_tree(BINARY_TREE_PTR node);

void _assert(const char *msg, const char *file, int line)
{
	printf("%s : %s %d", msg, file, line);
	exit(1);
}

int main()
{
	int i, numbers[MAX_TREE_NUMBERS]={81,56,43, 63,55,29,98,39,47,80,42,1,15,34,95,92,82,91,94,22};
	int del_numbers[MAX_DEL_NUMBERS]={98, 56, 42, 55, 39, 29}, del_number_index;
	BT_TEST root;
	BT_TEST_PTR search_node=NULL;
	int max_tree_numbers;
	
	if ( random_number_test )
	{
		del_number_index=0;
		max_tree_numbers = MAX_TREE_NUMBERS;
		srand ( time(NULL) );
	}
	else
	{
		max_tree_numbers = 20;
		del_number_index = 6;
	}
	
	InitBT_TestNode( &root, 50);
	
	/*insert into list*/
	printf("Inserting 20 numbers between 0 to 100\n");
	for(i=0;i<MAX_TREE_NUMBERS;)
	{
		int num;
		BT_TEST_PTR new_node;
		
		if ( random_number_test )
			num = rand()%100;
		else
			num = numbers[i];
		
		if ( (new_node = (BT_TEST_PTR ) malloc(sizeof(BT_TEST))) == NULL )
		{
			perror("malloc");
			return -1;
		}
		
		InitBT_TestNode(new_node, num);
		printf("Adding node %p (%d) : ", &new_node->t, num );
		if ( InsertNodeIntoBinaryTree(&root.t, &new_node->t ) != 0 )
			printf("failure\n");
		else
		{
			printf("success\n");
			i++;
			if ( random_number_test && !(i%3) && del_number_index<MAX_DEL_NUMBERS)
			{
				del_numbers[del_number_index] = num;
				del_number_index++;
			}
		}
		
	}
	print_tree(&root.t);
	printf("\n");
	del_number_index--;
	for(;del_number_index>=0;del_number_index--)
	{
		int result;
		BT_TEST del_node;
		InitBT_TestNode(&del_node, del_numbers[del_number_index]);
		
		printf("Searching %d : ", del_node.data);
		BINARY_TREE_PTR del = SearchBinaryTree(&root.t, &del_node.t);
		if ( del )
		{
			printf("found. Deleting it : ");
			RemoveNodeFromBinaryTree(del, NULL, NULL);
			if ( SearchBinaryTree(&root.t, &del_node.t) )
			{
				printf("node still exists\n");
				print_tree(&root.t);
				return 1;
			}
		}
		else
		{
			printf("***********Not found**************\n");
		}
	}
	printf("-------------------FINAL TREE---------------------------------------------\n");
	print_tree(&root.t);
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
	if (! IS_TREE_LIST_END(&node->left ) )
	{
		level++;
		printf("-");
		print_tree( TREE_LEFT_NODE(node) );
		level--;
	}
	
	if (! IS_TREE_LIST_END(&node->right) )
	{
		int j=0;
		printf("\n");
		
		for (j=0 ;j<(level*len);j++ ) printf(" ");
		printf("|\n");
		for (j=0 ;j<(level*len);j++ ) printf(" ");
		
		print_tree( TREE_RIGHT_NODE(node) );
	}
	
}
