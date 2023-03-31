#include <stddef.h>
#include "cl_core.h"
#include "cl_deque.h"
#include "cl_node.h"
#include "cl_tree_utils.h"
#include "cl_linked_binary_tree.h"

#define REQUIRED_NODE_FLAGS Node_flag(LEFT) | Node_flag(RIGHT))

#define DEFAULT_NODE Node_new(NA, 2, Node_attr(RIGHT), NULL, Node_attr(LEFT), NULL)

struct LinkedBinaryTree {
    Node * head;
    NodeAttributes * NA;
    size_t size;
    int (*compare) (Node_type(KEY), Node_type(KEY)); // must be NULL if KEY is not a flag in the tree
};

static Node * move(NodeAttributes * NA, Node * node, int dir) {
	if (dir == DIR_LEFT) {
		return Node_get(NA, node, LEFT);
	} else if (dir == DIR_RIGHT) {
		return Node_get(NA, node, RIGHT);
	}
	return NULL;
}

void LinkedBinaryTree_init(LinkedBinaryTree * lbt, int (*compare) (Node_type(KEY), Node_type(KEY)), NodeAttributes * NA) {
    lbt->head = NULL;
    lbt->NA = NA;
    lbt->size = 0;
    lbt->compare = compare;
}

LinkedBinaryTree * vLinkedBinaryTree_new(int (*compare) (Node_type(KEY), Node_type(KEY)), unsigned int node_flags, int narg_pairs, va_list args) {
    LinkedBinaryTree * lbt = (LinkedBinaryTree *) CL_MALLOC(sizeof(LinkedBinaryTree));
    if (!lbt) {
        return NULL;
    }

    node_flags |= Node_flag(VALUE); // must have at least the value component
    NodeAttributes * NA = vNodeAttributes_new(node_flags, narg_pairs, args);

    if (!NA) {
        CL_FREE(lbt);
        lbt = NULL;
        return NULL;
    }

    LinkedBinaryTree_init(lbt, compare, NA);

    return lbt;
}

LinkedBinaryTree * LinkedBinaryTree_new(int (*compare) (Node_type(KEY), Node_type(KEY)), unsigned int node_flags, int narg_pairs, ...) {
    va_list args;
    va_start(args, narg_pairs);
    LinkedBinaryTree * lbt = vLinkedBinaryTree_new(compare, node_flags, narg_pairs, args);
    va_end(args);
}

void Node_del_recursive(NodeAttributes * NA, Node * node) {
	Node_del_recursive(NA, Node_get(NA, node, LEFT));
	Node_set(NA, node, LEFT, NULL);
	Node_del_recursive(NA, Node_get(NA, node, RIGHT));
	Node_set(NA, node, RIGHT, NULL);
	Node_del(node);
}

void LinkedBinaryTree_del(LinkedBinaryTree * lbt) {
    Node_del_recursive(lbt->NA, lbt->head);
    lbt->head = NULL;
    if (lbt->NA) { // DEFAULT_NODE_ATTRIBUTES is statically allocated
        CL_FREE(lbt->NA);
    }
    lbt->NA = NULL;
    CL_FREE(lbt);
}

static bool LinkedBinaryTree_is_leaf(LinkedBinaryTree * lbt, Node * node) {
	if (node) {
		Node * child = move(lbt->NA, node, DIR_LEFT);
		if (!child) {
			child = move(lbt->NA, node, DIR_RIGHT);
			if (!child) {
				return true;
			}
		}
	}
	return false;
}

size_t LinkedBinaryTree_size(LinkedBinaryTree * lbt) {
	return lbt->size;
}

size_t LinkedBinaryTree_subtree_size(LinkedBinaryTree * lbt, Node * node) {
	if (node == lbt->head) {
		return lbt->size;
	}
	if (node) {
		return 1 + LinkedBinaryTree_subtree_size(lbt, move(lbt->NA, node, DIR_LEFT)) + LinkedBinaryTree_subtree_size(lbt, move(lbt->NA, node, DIR_RIGHT));
	}
	return 0;
}

// TODO:
int LinkedBinaryTree_path_to(Deque * path, LinkedBinaryTree * lbt, Node * node) {
	// try to do this without heap allocation of the Deque
}

int LinkedBinaryTree_rotate(LinkedBinaryTree * lbt, Node * node, int dir) {
	if (!node || !(dir == DIR_LEFT || dir == DIR_RIGHT)) {
		return CL_SUCCESS; // do nothing because it cannot be rotated. really should be InputError
	}
	
	Node * old_child = NULL;
	Node * parent = NULL;
	Node * new_child = NULL;
	bool has_parent = Node_has(lbt->NA, PARENT);
	if (has_parent) {
		parent = Node_get(lbt->NA, node, PARENT);
	} else {
		// TODO: traverse down to find node while keeping track of parent
	}
	
	if (dir == DIR_LEFT) {
		old_child = Node_get(lbt->NA, node, RIGHT);
		if (!old_child) { // cannot rotate left
			return CL_SUCCESS; // should probably be InputError
		}
		
		new_child = Node_get(lbt->NA, old_child, LEFT);
		Node_set(lbt->NA, node, RIGHT, new_child);
		Node_set(lbt->NA, old_child, LEFT, node);
		if (Node_get(lbt->NA, parent, LEFT) == node) {
			Node_set(lbt->NA, parent, LEFT, old_child);
		} else {
			Node_set(lbt->NA, parent, RIGHT, old_child);
		}

		if (has_parent) {
			Node_set(lbt->NA, new_child, PARENT, node);
			Node_set(lbt->NA, node, PARENT, old_child);
			Node_set(lbt->NA, old_child, PARENT, parent);
		}
	} else if (dir == DIR_RIGHT) {
		old_child = Node_get(lbt->NA, node, LEFT);
		if (!old_child) { // cannot rotate right
			return CL_SUCCESS; // should probably be InputError
		}
		
		new_child = Node_get(lbt->NA, old_child, RIGHT);
		Node_set(lbt->NA, node, LEFT, new_child);
		Node_set(lbt->NA, old_child, RIGHT, node);
		if (Node_get(lbt->NA, parent, LEFT) == node) {
			Node_set(lbt->NA, parent, LEFT, old_child);
		} else {
			Node_set(lbt->NA, parent, RIGHT, old_child);
		}

		if (has_parent) {
			Node_set(lbt->NA, new_child, PARENT, node);
			Node_set(lbt->NA, node, PARENT, old_child);
			Node_set(lbt->NA, old_child, PARENT, parent);
		}
	}
	
	return lbt;
}

LinkedBinaryTree * LinkedBinaryTree_split_rotate(LinkedBinaryTree * lbt, size_t index, int dir) {
    size_t N = lbt->size;
    if (index >= N || lbt->nodes[index] == NULL) {
        return lbt; // do nothing because it cannot be rotated or rotating is meaningless
	}
    size_t left_child = move(index, DIR_LEFT);
    size_t right_child = move(index, DIR_RIGHT);
    
    if (dir == DIR_LEFT) {
        size_t grandchild = move(right_child, DIR_LEFT);
        if (right_child >= N || grandchild >= N || lbt->nodes[right_child] == NULL || lbt->nodes[grandchild] == NULL) { // cannot rotate left
            return lbt;
		}
        // move left_child to its left subtree // might incur O(N) memory increase and build time
        LinkedBinaryTree_move_subtree(lbt, left_child, move(left_child, DIR_LEFT));
        
        // swap index and left_child
        LinkedBinaryTree_swap(lbt, index, left_child);
        
        // swap grandchild and index
        LinkedBinaryTree_swap(lbt, index, grandchild);
        
        // move grandchild's left subtree to left_child's right subtree
        LinkedBinaryTree_move_subtree(lbt, move(grandchild, DIR_LEFT), move(left_child, DIR_RIGHT));
        
        // move granchild's right subtree to right_child's left subtree
        LinkedBinaryTree_move_subtree(lbt, move(grandchild, DIR_RIGHT), move(right_child, DIR_LEFT));
	} else if (dir == DIR_RIGHT) {
        size_t grandchild = move(left_child, DIR_RIGHT);
        if (left_child >= N || grandchild >= N || lbt->nodes[left_child] == NULL || lbt->nodes[grandchild] == NULL) { // cannot rotate right
            return lbt;
		}
        if (grandchild >= N) {
            LinkedBinaryTree_resize(lbt, grandchild+1);
		}
        
        // move right_child to its right subtree // might incur O(N) memory increase and build time
        LinkedBinaryTree_move_subtree(lbt, right_child, move(right_child, DIR_RIGHT));
        // swap index and right_child
        LinkedBinaryTree_swap(lbt, index, right_child);
        // swap grandchild and index
        LinkedBinaryTree_swap(lbt, index, grandchild);
        // move granchild's right subtree to right_child's left subtree
        LinkedBinaryTree_move_subtree(lbt, move(grandchild, DIR_RIGHT), move(right_child, DIR_LEFT));
        // move grandchild's left subtree to left_child's right subtree
        LinkedBinaryTree_move_subtree(lbt, move(grandchild, DIR_LEFT), move(left_child, DIR_RIGHT));
        
    } else {
		assert(dir == DIR_LEFT || dir == DIR_RIGHT);
        // handle error
	}
    
    return lbt;
}

/*
// Tree copy is owned by caller
lBTNode * lBT_copy(lBTNode * node) {
	if (!node) {
		return NULL;
	}
	lBTNode * out = lBTNode_create(node->data);
		
	if (out->left = lBT_copy(node->left)) {
		out->left->parent = out;
	}
	if (out->right = lBT_copy(node->right)) {
		out->right->parent = out;
	}
	return out;
}

unsigned int lBT_height(lBTNode * root) {
	if (!root) {
		return 0;
	}
	int left_height = lBT_height(root->left);
	int right_height = lBT_height(root->right);
	return (left_height > right_height ? left_height : right_height) + 1;
}

// frees a single node after removing it from any tree
// note there is a high risk of memory leak in this method. It is only really meant to be used in conjunction with lBTNode_destroy
//  if a node that is not a leaf is attempted to be deleted, memory leak as children will not be properly destroyed
//  if data is "owned" by the tree and the flag free_contents is not enabled, memory leak
void lBTNode_delete(lBTNode * node, int free_contents) {
	DEBUG_PRINT(("deleting node at %p\n", node));
	if (free_contents) {
		DEBUG_PRINT(("freeing node contents at %p\n", node->data));
		free(node->data);
	}
	node->data = NULL; // if free_contents is 0 and node->data is malloc'd and not free'd before calling this function, it WILL cause a memory leak
	if (node->parent) {
		if (node->parent->left == node) {
			node->parent->left = NULL;
		} else if (node->parent->right == node) {
			node->parent->right = NULL;
		} // other conditions would exist if this was an N-ary tree with indexed children
	}
	node->parent = NULL;
	node->left = NULL;
	node->right = NULL;
	free(node);
}

// frees all memory associated with the tree. If the data contents are owned by the tree, free_contents MUST be != 0
// probably could be implemented as a post-order traversal of lBT_delete but would have to change int free_contents to void * free_contents and cast as (int *)
void lBT_destroy(lBTNode * node, int free_contents) {
	DEBUG_PRINT(("Destroying tree node at %p\n", node));
	if (!node) {
		return;
	} 
	if (node->left) {
		lBT_destroy(node->left, free_contents);
	}
	if (node->right) {
		lBT_destroy(node->right, free_contents);
	}	
	lBTNode_delete(node, free_contents);
	return;
}

//this only swaps a parent with one of its children. It does not do a general node swap (yet)
// TODO: implement checks to ensure swapping handles both the case of swapping the nodes that are not parent/child
// TODO: I think it is far easier to just swap the data...
void lBT_swap(lBTNode * node1, lBTNode * node2) {
	void * temp = node1->data;
	node1->data = node2->data;
	node2->data = temp;
}

// TODO rework for general insert
// TODO need to validate the logic for inserting in a location that already has a node
void lBT_insert(lBTNode * parent, lBTNode * node, unsigned int child_index) {
	if (!node) {
		return;
	}
	lBTNode * child = NULL;
	if (child_index) { // insert to right in parent
		child = parent->right;
		parent->right = node;
	} else { // insert to left in parent
		child = parent->left;
		parent->left = node;
	}
	
	size_t a = 0, b = 0;
	if (child) {
		child->parent = node;
		if (child->right) {
			b = child->right->subTreeSize;
		}
		if (child->left) {
			a = child->left->subTreeSize;
		}
	}
	
	if (b >= a) {
		node->right = child;
	} else {
		node->left = child;
	}
	node->parent = parent;
	while (parent) {
		parent = parent->parent;
	}
}

void * lBT_remove(lBTNode * node) {
	if (!node) {
		return NULL;
	}
	void * result = node->data;
	
	lBTNode * parent = node->parent;
	int next_dir = node == parent->right ? 1 : 0;
	int dir = 0;
	lBTNode * left = node->left, * right = node->right;
	lBTNode * child = NULL;
	
	// while there are two children. It is easy to do this recursively, but the memory cost is would be a high number * log(node->subTreeSize). This way, O(1)
	while (left && right) {
		dir = next_dir;
		// lift the left/right child with the largest subtree then update left/right pointers
		if (right->subTreeSize >= left->subTreeSize) {
			child = right;
			left->parent = child;
			child->parent = parent;
			right = left;
			left = child->left;
			child->left = right;
			right = child->right;
			next_dir = 1;
		} else {
			child = left;
			right->parent = child;
			child->parent = parent;
			left = right;
			right = child->right;
			child->right = left;
			left = child->left;
			next_dir = 0;
		}
		if (parent) {
			if (dir) {
				parent->right = child;
			} else {
				parent->left = child;
			}
		}
		parent = child;		
	}
	dir = next_dir;
	if (left) {
		child = left;
	} else {
		child = right;
	}
	if (child) {
		child->parent = parent;
	}
	if (parent) {
		if (dir) {
			parent->right = child;
		} else {
			parent->left = child;
		}
	}
	
	// work way back up the tree updating subTreeSizes
	while (parent) {
		parent->subTreeSize = 1;
		if (parent->left) {
			parent->subTreeSize += parent->left->subTreeSize;
		}
		if (parent->right) {
			parent->subTreeSize += parent->right->subTreeSize;
		}
		parent = parent->parent;
	}
	// delete the tree node but do not delete the contents (leave that to the caller)
	lBTNode_delete(node, 0); 
}

// *************************************************** TRAVERSALS & ASSOCIATED FUNCTIONS

size_t lBT_preorder(lBTNode * root, int (*func)(void *, void *), void * func_input) {
	if (!root) {
		return 0;
	}
	// for a Binary Tree of height h, should not have to allocate more space than 2*height + 1 (largest height both children + 1 extra) since this is DFS
	// the Deque only needs to satisfy the conditions for a Stack
	// Stack based on underlying Deque would not actually save any memory
	Deque * deq = Deque_create(root->subTreeSize/2);
	size_t count = 0;
	Deque_push_back(deq, (void *) root);
	int cont = 1;
	while (cont && !Deque_is_empty(deq)) {
		lBTNode * node = (lBTNode *) Deque_pop_back(deq);
		count++;
		cont = !func(func_input, (void *)node);
		if (node->right) {
			Deque_push_back(deq, (void *) node->right);
		}
		if (node->left) {
			Deque_push_back(deq, (void *) node->left);
		}
	}
	
	Deque_destroy(deq, 0); // destroy the Deque but not its contents
	
	return count;
}

// make generic for paired values
define_Named_Tuple(BTN_visited,lBTNode, *node, int, val)

size_t lBT_postorder(lBTNode * root, int (*func)(void *, void *), void * func_input) {
	if (!root) {
		return 0;
	}
	
	// algorithm outline...
	// Deque of a pair (lBTNode *node, int val)
	// node is not null and value is 0 or 1:
	// if value is 1, node's children already added so visit node
	// if value is 0, push it back on Stack with a 1 and add right child then left child with 0s
	Deque * nodes = Deque_create(root->subTreeSize/2);
	
	BTN_visited * pair = (BTN_visited *) malloc(sizeof(BTN_visited));
	*pair = (BTN_visited) {root, 0};
	BTN_visited * node;
	
	Deque_push_back(nodes, (void *) pair);
	
	size_t count = 0;
	int cont = 1;
	while (cont && !Deque_is_empty(nodes)) {
		node = (BTN_visited *) Deque_pop_back(nodes);
		if (node->val) {
			cont = !func(func_input, (void *)node->node);
			count++;
			free(node);
		} else {
			node->val = 1;
			Deque_push_back(nodes, (void *) node);
			if (node->node->right) {
				pair = (BTN_visited *) malloc(sizeof(BTN_visited));
				*pair = (BTN_visited) {node->node->right, 0};
				Deque_push_back(nodes, (void *) pair);
			}
			if (node->node->left) {
				pair = (BTN_visited *) malloc(sizeof(BTN_visited));
				*pair = (BTN_visited) {node->node->left, 0};
				Deque_push_back(nodes, (void *) pair);
			}
		}
	}
	
	Deque_destroy(nodes, 1); // destroy all nodes and free their contents
	
	return count;
}

size_t lBT_inorder(lBTNode * root, int (*func)(void *, void *), void * func_input) {
	if (!root) {
		return 0;
	}
	
	// algorithm outine...
	// Deque of a pair (lBTNode *node, int val)
	// node is not null and value is 0 or 1:
	// if value is 1, node's children already added so visit node
	// if value is 0, push it back on Stack with a 1 and add right child then left child with 0s
	Deque * nodes = Deque_create(root->subTreeSize/2);
	
	BTN_visited * pair = (BTN_visited *) malloc(sizeof(BTN_visited));
	*pair = (BTN_visited) {root, 0};
	BTN_visited * node;
	
	Deque_push_back(nodes, (void *) pair);
	
	size_t count = 0;
	int cont = 1;
	while (cont && !Deque_is_empty(nodes)) {
		node = (BTN_visited *) Deque_pop_back(nodes);
		if (node->val) {
			cont = !func(func_input, (void *)node->node);
			count++;
			free(node);
		} else {
			node->val = 1;
			if (node->node->right) {
				pair = (BTN_visited *) malloc(sizeof(BTN_visited));
				*pair = (BTN_visited) {node->node->right, 0};
				Deque_push_back(nodes, (void *) pair);
			}
			Deque_push_back(nodes, (void *) node);
			if (node->node->left) {
				pair = (BTN_visited *) malloc(sizeof(BTN_visited));
				*pair = (BTN_visited) {node->node->left, 0};
				Deque_push_back(nodes, (void *) pair);
			}
		}
	}
	
	Deque_destroy(nodes, 1); // destroy all BTN_visited nodes and free their contents
	
	return count;
}

size_t lBT_level_order(lBTNode * root, int (*func)(void *, void *), void * func_input) {
	Deque * nodes = Deque_create(root->subTreeSize/2);
	
	Deque_push_back(nodes, (void *) root);
	
	size_t count = 0;
	int cont = 1;
	while (cont && !Deque_is_empty(nodes)) {
		lBTNode * node = (lBTNode *) Deque_pop_front(nodes);
		if (node->left) {
			Deque_push_back(nodes, (void *) node->left);
		}
		if (node->right) {
			Deque_push_back(nodes, (void *) node->right);
		}
		count++;
		cont = !func(func_input, (void *)node);
	}
	
	Deque_destroy(nodes, 0); // destroy the Deque but not its contents
	return count;
}

// TODO need to test...move to lHeap
// TODO I am pretty sure there's a bug here. When the final level is full, this will just return NULL. Yeah, I did this incorrectly...I need to check the rightmost side first on tree_height-1
lBTNode * lCompleteBinTree_get_end_parent(lBTNode * root, unsigned int tree_height) {
	if (tree_height && !root->left && !root->right) {
		return root;
	}
	
	lBTNode * child = lCompleteBinTree_get_end_parent(root->left, tree_height-1);
	if (!child) {
		return child;
	}
	child = lCompleteBinTree_get_end_parent(root->right, tree_height-1);
	if (!child) {
		return child;
	}
	
	return NULL;
}

void ** lBT_to_array(lBTNode * root, size_t data_size, size_t * return_size) {
	// not yet implemented. Need to test with multiple array types
	
	unsigned int height = lBT_height(root);
	DEBUG_PRINT(("Tree height = %u\n", height));
	if (!height) {
		DEBUG_PRINT(("0 height found\n"));
		*return_size = 0;
		return NULL;
	}
	*return_size = 1;
	while (height--) {
		*return_size *= 2;
	}
	(*return_size)--;
	
	DEBUG_PRINT(("output array size = %llu\n", *return_size));
	
	size_t cur_index = 0;
	size_t layer_end_index = 0;
	size_t insert_index = 0;
	
	void ** result = (void **) malloc(*return_size*data_size);
	// level_order_traversal to get array
	
	// use result as the stack in place by casting lBTNode* to void*
	result[insert_index++] = (void *) root;
	lBTNode * node;
	void * data;
	while (cur_index < *return_size) {
		if (cur_index == layer_end_index && insert_index < *return_size - 1) {
			// add the next layer
			DEBUG_PRINT(("Adding next layer using nodes in indices from %llu to %llu\n", layer_end_index, insert_index));
			size_t end = insert_index;
			while (layer_end_index < end && insert_index < *return_size - 1) {
				if (result[layer_end_index]) {
					node = (lBTNode *) result[layer_end_index];
					result[insert_index++] = (void *) node->left;
					result[insert_index++] = (void *) node->right;
				} else {
					result[insert_index++] = NULL;
					result[insert_index++] = NULL;
				}
				layer_end_index++;
			}
			DEBUG_PRINT(("Next layer ends at %llu\n", insert_index));
		}
		
		DEBUG_PRINT(("Grabbing node from %llu and replacing with data\n", cur_index));
		if (result[cur_index]) {
			// replace current position with data
			data = ((lBTNode *) result[cur_index])->data;
			memcpy(result[cur_index], data, data_size);
			DEBUG_PRINT(("copied data at %p\n", data));
		} // else the node is NULL and so keep it NULL
		cur_index++;
	}
	
	return result;
}

// helper for _array_to_linked_binary_tree for recursive calls
lBTNode * _array_to_linked_binary_tree(void ** arr, size_t index, size_t num) {
	if (index >= num || !arr[index]) {
		return NULL;
	}
	
	lBTNode * node = lBTNode_create(arr[index]);
	if (node->left = _array_to_linked_binary_tree(arr, index*2+1, num)) {
		node->left->parent = node;
	}
	if (node->right = _array_to_linked_binary_tree(arr, index*2+2, num)) {
		node->right->parent = node;
	}
	return node;
}

// Tree is owned by caller
lBTNode * array_to_linked_binary_tree(void ** arr, size_t num) {
	return _array_to_linked_binary_tree(arr, 0, num);
}

*/