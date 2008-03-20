#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ds/avl_tree.h>
struct bt_test
{
	AVL_TREE t;
	int data;
};
typedef struct bt_test BT_TEST, * BT_TEST_PTR;

COMPARISION_RESULT compare_number(struct binary_tree * node1, struct binary_tree * node2);
BT_TEST_PTR InitBT_TestNode(BT_TEST_PTR node, int data);

void print_tree(AVL_TREE_PTR avl_node);
int * init_numbers(int * total_numbers, int ** del_numbers_ptr, int * total_del_numbers);
int parse_arguments(int argc, char * argv[]);

extern int verbose_level;
int main(int argc, char * argv[])
{
	BT_TEST first_element;
	AVL_TREE_PTR root=&first_element.t;
	BT_TEST_PTR search_node=NULL;
	int i, * numbers, total_numbers, * del_numbers, del_number_index;
	
	
	if ( parse_arguments(argc, argv) )
		return;
	numbers = init_numbers(&total_numbers, &del_numbers, &del_number_index);
	
	InitBT_TestNode( &first_element, numbers[0]);
	
	/*insertion test*/
	if ( verbose_level > 0 ) printf("Inserting %d numbers between 0 to 100\n", total_numbers);
	for(i=1;i<total_numbers; i++)
	{
		BT_TEST_PTR new_node;
		if ( (new_node = (BT_TEST_PTR ) malloc(sizeof(BT_TEST))) == NULL )
		{
			perror("malloc");
			return -1;
		}
		InitBT_TestNode(new_node, numbers[i]);
		
		if ( verbose_level > 1 ) printf("Adding node %p (%d) : ", &new_node->t, numbers[i] );
		
		if ( InsertNodeIntoAvlTree( &root, &new_node->t ) != 0 )
		{
			printf("failure\n");
			return;
		}
		else
			if ( verbose_level > 1 ) printf("success\n");
		
	}
	if ( verbose_level > 1 )
	{
		printf("After insertion - Tree :\n");
		print_tree(root);
	}
	if ( verbose_level > 0 ) printf("\nDeleting %d numbers from the tree\n", del_number_index);
	
	
	/*deletion test*/
	del_number_index--;
	for(;del_number_index>=0;del_number_index--)
	{
		int result;
		BT_TEST del_node;
		InitBT_TestNode(&del_node, del_numbers[del_number_index]);
		
		if ( verbose_level > 1 ) printf("Searching %d : ", del_node.data);
		AVL_TREE_PTR del = SearchAvlTree( root, &del_node.t);
		if ( del )
		{
			if ( verbose_level > 1 ) printf("found. Deleting it : ");
			RemoveNodeFromAvlTree( &root, del);
			if ( SearchAvlTree( root, &del_node.t) )
			{
				printf("Deleted node still exists(%d)\n", del_number_index);
				print_tree( root );
				return 1;
			}
			if ( verbose_level > 1 ) printf("Sucess\n");
		}
		else
		{
			printf("Node (%d) not found while deleting \n", del_node.data);
			return 1;
		}
	}
	if ( verbose_level > 1 )
	{
		printf("-------------------FINAL TREE---------------------------------------------\n");
		print_tree( root);
		printf("\n--------------------------------------------------------------------------\n");
	}
	if ( verbose_level > 0 )
		printf("\nNormal exit\n");
	return 0;
}

BT_TEST_PTR InitBT_TestNode(BT_TEST_PTR node, int data)
{
	InitAvlTreeNode(&node->t, compare_number);

	node->data = data;
	return node;
}


void print_tree(AVL_TREE_PTR avl_node)
{
	static int i=0, level=0;
	BINARY_TREE_PTR node = &avl_node->bintree;
	int len = printf("%02d(%d)", STRUCT_FROM_MEMBER(BT_TEST_PTR, t, node)->data, avl_node->height )+1;
	if (! IS_TREE_LIST_END(&node->right) )
	{
		level++;
		printf("-");
		print_tree( (AVL_TREE_PTR) TREE_RIGHT_NODE(node) );
		level--;
	}
	
	if (! IS_TREE_LIST_END(&node->left ) )
	{
		int j=0;
		printf("\n");
		
		for (j=0 ;j<(level*len);j++ ) printf(" ");
		printf("|\n");
		for (j=0 ;j<(level*len);j++ ) printf(" ");
		
		print_tree( (AVL_TREE_PTR) TREE_LEFT_NODE(node) );
	}
	
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
