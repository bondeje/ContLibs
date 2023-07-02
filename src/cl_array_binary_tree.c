#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>
#include <math.h>
#include "cl_core.h"
#include "cl_node.h"
#include "cl_tree_utils.h"
#include "cl_circular_buffer.h"
#include "cl_tuple.h"
#include "cl_array_binary_tree.h"

// TODO: need to fix ArrayBinaryTree_path_to when ArrayBinaryTree_traverse is available
 
#define DEFAULT_SCALE_FACTOR 2

#define REQUIRED_NODE_FLAGS 0
#define DEFAULT_NODE Node_new(NA, 0)

//#define DEFAULT_NODE_ATTRIBUTES NodeAttributes_new(REQUIRED_NODE_FLAGS, 1, Node_attr(VALUE), NULL)

// aliases
#define move array_move_index

define_Named_Tuple(stsrc_stdest, size_t, src, size_t, dest)
define_Named_Tuple(treevalue_deque, void*, value, Deque*, deq);
define_Named_Tuple(max_min_st, size_t, max, size_t, min);

struct ArrayBinaryTree {
    Node ** nodes;
    size_t capacity;
    NodeAttributes * NA;
    size_t size;
    int (*compare) (Node_type(KEY), Node_type(KEY));
};

void ArrayBinaryTree_init(ArrayBinaryTree * abt, size_t capacity, int (*compare) (Node_type(KEY), Node_type(KEY)), NodeAttributes * NA) {
    for (size_t i = 0; i < capacity; i++) {
        abt->nodes[i] = NULL;
    }
    abt->capacity = capacity;
    abt->NA = NA;
    abt->size = 0;
    abt->compare = compare;
}
ArrayBinaryTree * vArrayBinaryTree_new(size_t capacity, int (*compare) (Node_type(KEY), Node_type(KEY)), unsigned int node_flags, int narg_pairs, va_list args) {
	ArrayBinaryTree * abt = (ArrayBinaryTree *) CL_MALLOC(sizeof(ArrayBinaryTree));
    if (!abt) {
        return NULL;
    }

    abt->nodes = (Node **) CL_MALLOC(sizeof(Node*) * capacity);
    if (!abt->nodes) {
        CL_FREE(abt);
        abt = NULL;
        return NULL;
    }

	node_flags |= REQUIRED_NODE_FLAGS; // must have at least the value component
	
	NodeAttributes * NA = vNodeAttributes_new(node_flags, narg_pairs, args);

    if (!NA) {
        CL_FREE(abt->nodes);
        abt->nodes = NULL;
        CL_FREE(abt);
        abt = NULL;
        return NULL;
    }

    ArrayBinaryTree_init(abt, capacity, compare, NA);

    return abt;
}

ArrayBinaryTree * ArrayBinaryTree_new(size_t capacity, int (*compare) (Node_type(KEY), Node_type(KEY)), unsigned int node_flags, int narg_pairs, ...) {
    va_list args;
	va_start(args, narg_pairs);
	ArrayBinaryTree * abt = vArrayBinaryTree_new(capacity, compare, node_flags, narg_pairs, args);
	va_end(args);

	return abt;
}

void ArrayBinaryTree_del(ArrayBinaryTree * abt) {
    for (size_t i = 0; i < abt->capacity; i++) {
        if (abt->nodes[i]) {
            Node_del(abt->nodes[i]);
            abt->nodes[i] = NULL;
        }
    }
    CL_FREE(abt->nodes);
    abt->nodes = NULL;
    if (abt->NA) { // DEFAULT_NODE_ATTRIBUTES is statically allocated
        CL_FREE(abt->NA);
    }
    abt->NA = NULL;
    CL_FREE(abt);
}

static bool ArrayBinaryTree_is_leaf(ArrayBinaryTree * abt, size_t index) {
	if (index < abt->size && abt->nodes[index] != NULL) {
		size_t child = move(index, DIR_LEFT);
		if (child >= abt->size || abt->nodes[child] == NULL) {
			child = move(index, DIR_RIGHT);
			if (child >= abt->size || abt->nodes[child] == NULL) {
				return true;
			}
		}
	}
	return false;
}

size_t ArrayBinaryTree_size(ArrayBinaryTree * abt) {
	return abt->size;
}

size_t ArrayBinaryTree_subtree_size(ArrayBinaryTree * abt, size_t index) {
	if (!index) {
		return abt->size;
	} else {
		if (index >= abt->size || abt->nodes[index] == NULL) {
			return 0;
		} else {
			return 1 + ArrayBinaryTree_subtree_size(abt, move(index, DIR_LEFT)) + ArrayBinaryTree_subtree_size(abt, move(index, DIR_RIGHT));
		}
	}
}

static void ArrayBinaryTree_diameter_helper(ArrayBinaryTree * abt, size_t index, size_t * max_diameter, size_t *height) {
	if (index >= abt->size || abt->nodes[index] == NULL) {
		return;
	}
	size_t l_max_diameter = 0;
	size_t l_height = 0;
	size_t r_max_diameter = 0;
	size_t r_height = 0;
	ArrayBinaryTree_diameter_helper(abt, move(index, DIR_LEFT), &l_max_diameter, &l_height);
	ArrayBinaryTree_diameter_helper(abt, move(index, DIR_RIGHT), &r_max_diameter, &r_height);
	size_t new_diameter = 1 + l_height + r_height;
	
	if (l_max_diameter > r_max_diameter) {
		if (l_max_diameter < new_diameter) {
			*max_diameter = new_diameter;
		} else {
			*max_diameter = l_max_diameter;
		}
	} else {
		if (r_max_diameter < new_diameter) {
			*max_diameter = new_diameter;
		} else {
			*max_diameter = r_max_diameter;
		}
	}
	
	*height = 1 + (l_height > r_height ? l_height : r_height);
}

size_t ArrayBinaryTree_diameter(ArrayBinaryTree * abt, size_t index) {
	if (index >= abt->size || abt->nodes[index] == NULL) {
		return 0;
	}
	size_t max_diameter = 0;
	size_t height = 0;
	ArrayBinaryTree_diameter_helper(abt, index, &max_diameter, &height);
	return max_diameter;
}

size_t ArrayBinaryTree_depth(ArrayBinaryTree * abt, size_t index) {
	if (index >= abt->size || abt->nodes[index] == NULL) {
		return 0;
	}
	size_t depth = 0;
	size_t parent = move(index, DIR_PARENT);
	while (parent < index) {
		index = parent;
		parent = move(index, DIR_PARENT);
		depth += 1;
	}
	return depth;
}

size_t ArrayBinaryTree_height(ArrayBinaryTree * abt, size_t index) {
	if (index >= abt->size || abt->nodes[index] == NULL) {
		return 0;
	}
	size_t max_left = ArrayBinaryTree_height(abt, move(index, DIR_LEFT));
	size_t max_right = ArrayBinaryTree_height(abt, move(index, DIR_RIGHT));
	return 1 + (max_left > max_right ? max_left : max_right);
}

size_t ArrayBinaryTree_approx_height(ArrayBinaryTree * abt) {
	return (size_t) log2(abt->size);
}

static void ArrayBinaryTree_swap(ArrayBinaryTree * abt, size_t index1, size_t index2) {
	Node ** temp;
    cl_swap_buffered(abt->nodes + index1, abt->nodes + index2, sizeof(Node *), temp);
}

static void ArrayBinaryTree_swap_value(ArrayBinaryTree * abt, size_t index1, size_t index2) {
    if (!(index1 < abt->size) || !(index2 < abt->size)) {
        return;
    }
    void ** temp;
    cl_swap_buffered(&Node_get(abt->NA, abt->nodes[index1], VALUE), &Node_get(abt->NA, abt->nodes[index1], VALUE), sizeof(Node_type(VALUE)), temp);
}

static int ArrayBinaryTree_resize(ArrayBinaryTree * abt, size_t capacity) { 
    capacity = (capacity >= abt->size) ? capacity : abt->size; // do not shrink below current size
    Node ** new_nodes = CL_REALLOC(abt->nodes, sizeof(Node*) * capacity);
    if (!new_nodes) {
        return ABT_REALLOC_FAILURE;
    }
    abt->nodes = new_nodes;
    for (size_t i = abt->size; i < capacity; i++) {
        abt->nodes[i] = NULL;
    }
    abt->capacity = capacity;
	return ABT_SUCCESS;
}

void ArrayBinaryTree_move_subtree(ArrayBinaryTree * abt, size_t root, size_t target) {
	size_t N = abt->size;
	if (root >= N || abt->nodes[root] == NULL) {
		return;
	}
	
	if (target > root) { // shift down or parallel right
		size_t Ncur = N;
		size_t init_cap = 2*ArrayBinaryTree_approx_height(abt);
        Deque * st_visit = Deque_new(init_cap); // holding pair_stsrc_stdest
		Deque * st_traverse = Deque_new(init_cap); // holding pair_stsrc_stdest
		Deque_push_back(st_traverse, stsrc_stdest_new(root, target));
		
		// find if target is in the left subtree or to the left of root
		size_t parent = target;
		size_t left = move(root, DIR_LEFT);
		size_t rightmost_left_level = next_pow2(left+1)-2;
		while (parent > rightmost_left_level) {
			parent = move(parent, DIR_PARENT); // rightmost_left_level can never be root so we do not have to worry about the 0-->SIZE_MAX issue in _move
		}
		int dir_leader = DIR_LEFT;
		int dir_follower = DIR_RIGHT;
		if (parent > left) { // target is to the right of the root
			int dir_leader = DIR_RIGHT;
			int dir_follower = DIR_LEFT;
		}
		while (Deque_size(st_traverse)) {
			stsrc_stdest * ttail = Deque_peek_back(st_traverse);
			if (ttail->src < N && abt->nodes[ttail->src]) {
				Deque_push_back(st_visit, Deque_pop_back(st_traverse));
				
				root = move(ttail->src, dir_follower); // move to left child of source
				target = move(ttail->dest, dir_follower); // move to left child of destination
				Deque_push_back(st_traverse, stsrc_stdest_new(root, target));
				
				root = move(ttail->src, dir_leader); // move to right child of source
				target = move(ttail->dest, dir_leader); // move to rit child of destination
				Deque_push_back(st_traverse, stsrc_stdest_new(root, target)); // add right children to traversal stack
			} else {
				stsrc_stdest_del(Deque_pop_back(st_traverse));
				ttail = Deque_peek_back(st_traverse);
				stsrc_stdest * vtail = Deque_peek_back(st_visit); // might be NULL if empty
				while (Deque_size(st_visit) && (!Deque_size(st_traverse) || vtail->src > ttail->src)) {
					vtail = Deque_pop_back(st_visit);
					if ((vtail->dest) >= Ncur) {
						// this next line is why I cannot do a traditional traversal, which is stable only if the underlying tree is NOT modified
						ArrayBinaryTree_resize(abt, vtail->dest + 1);
						Ncur = abt->size;
					}
					ArrayBinaryTree_swap(abt, vtail->src, vtail->dest);
					stsrc_stdest_del(vtail);
				}
			}
		}
        while (Deque_size(st_visit)) {
            stsrc_stdest_del(Deque_pop_back(st_visit));
        }
		Deque_del(st_visit);
		Deque_del(st_traverse);
	} else if (target < root) { // shift up or parallel left
		if (target < 0) {
			assert(target >= 0);
			// handle error
		}
		
		// perform a preorder traversal of the tree moving subtree elements at root to subtree rooted at target
		size_t init_cap = 2*ArrayBinaryTree_approx_height(abt);
		Deque * st_traverse = Deque_new(init_cap); // holding pair_stsrc_stdest
		Deque_push_back(st_traverse, stsrc_stdest_new(root, target));
		while (Deque_size(st_traverse)) {
			stsrc_stdest * ttail = Deque_pop_front(st_traverse);
			if (ttail->src < abt->size && abt->nodes[ttail->src] != NULL) {
				ArrayBinaryTree_swap(abt, ttail->src, ttail->dest); // swap source and destination
				
				root = move(ttail->src, DIR_RIGHT); // move to the right child of source
				target = move(ttail->dest, DIR_RIGHT); // move to the right child of destination
				Deque_push_back(st_traverse, stsrc_stdest_new(root, target));
				
				root = move(ttail->src, DIR_LEFT); // move to the left child of source
				target = move(ttail->dest, DIR_LEFT); // move to the left child of destination
				Deque_push_back(st_traverse, stsrc_stdest_new(root, target));
			}
			stsrc_stdest_del(ttail);
		}
		Deque_del(st_traverse); // do not have to dealloc anything since the Deque should be empty to reach this point
	}
	// else do nothing and return
}

int ArrayBinaryTree_rotate(ArrayBinaryTree * abt, size_t index, int dir) {
	size_t N = abt->size;
	if (index >= N || abt->nodes[index] != NULL || !(dir == DIR_LEFT || dir == DIR_RIGHT)) {
		return CL_SUCCESS; // do nothing because it cannot be rotated. really should be InputError
	}
	
	size_t left_child = move(index, DIR_LEFT);
	size_t right_child = move(index, DIR_RIGHT);
	
	if (dir == DIR_LEFT) {
		if (right_child >= N || abt->nodes[right_child] == NULL) { // cannot rotate left
			return CL_SUCCESS;
		}
		
		// move index's left child subtree down to its left // down and left move
		ArrayBinaryTree_move_subtree(abt, left_child, move(left_child, DIR_LEFT));
		
		// swap index to its left child position
		ArrayBinaryTree_swap(abt, index, left_child);
		
		// move index's right child's left subtree to index's left child's right subtree // left move
		ArrayBinaryTree_move_subtree(abt, move(right_child, DIR_LEFT), move(left_child, DIR_RIGHT));
		
		// swap index's right child to index
		ArrayBinaryTree_swap(abt, index, right_child);
		
		// move index's right child's right subtree up to index's right subtree // up and left move
		ArrayBinaryTree_move_subtree(abt, move(right_child, DIR_RIGHT), right_child);
	} else if (dir == DIR_RIGHT) {
		if (left_child >= N || abt->nodes[left_child] == NULL) { // cannot rotate right
			return CL_SUCCESS;
		}
		// if rotating right, the swaps require AT LEAST a tree of size right_child + 1
		if (right_child >= N) {
			ArrayBinaryTree_resize(abt, right_child+1);
		}
		// move index's right child down to its right // down and right move
		ArrayBinaryTree_move_subtree(abt, right_child, move(right_child, DIR_RIGHT));
		
		// swap index to its right child position
		ArrayBinaryTree_swap(abt, index, right_child);
		
		// move index's left child's right subtree to index's right child's left subtree // right move
        ArrayBinaryTree_move_subtree(abt, move(left_child, DIR_RIGHT), move(right_child, DIR_LEFT));
		
		// swap index's left child to index
        ArrayBinaryTree_swap(abt, index, left_child);
        
        // move index's left child's left subtree up to index's left subtree // up and right
        ArrayBinaryTree_move_subtree(abt, move(left_child, DIR_LEFT), left_child);
	}
	
	return CL_SUCCESS;
}

int ArrayBinaryTree_split_rotate(ArrayBinaryTree * abt, size_t index, int dir) {
    size_t N = abt->size;
    if (index >= N || abt->nodes[index] == NULL) {
        return CL_SUCCESS; // do nothing because it cannot be rotated or rotating is meaningless
	}
    size_t left_child = move(index, DIR_LEFT);
    size_t right_child = move(index, DIR_RIGHT);
    
    if (dir == DIR_LEFT) {
        size_t grandchild = move(right_child, DIR_LEFT);
        if (right_child >= N || grandchild >= N || abt->nodes[right_child] == NULL || abt->nodes[grandchild] == NULL) { // cannot rotate left
            return CL_SUCCESS;
		}
        // move left_child to its left subtree // might incur O(N) memory increase and build time
        ArrayBinaryTree_move_subtree(abt, left_child, move(left_child, DIR_LEFT));
        
        // swap index and left_child
        ArrayBinaryTree_swap(abt, index, left_child);
        
        // swap grandchild and index
        ArrayBinaryTree_swap(abt, index, grandchild);
        
        // move grandchild's left subtree to left_child's right subtree
        ArrayBinaryTree_move_subtree(abt, move(grandchild, DIR_LEFT), move(left_child, DIR_RIGHT));
        
        // move granchild's right subtree to right_child's left subtree
        ArrayBinaryTree_move_subtree(abt, move(grandchild, DIR_RIGHT), move(right_child, DIR_LEFT));
	} else if (dir == DIR_RIGHT) {
        size_t grandchild = move(left_child, DIR_RIGHT);
        if (left_child >= N || grandchild >= N || abt->nodes[left_child] == NULL || abt->nodes[grandchild] == NULL) { // cannot rotate right
            return CL_SUCCESS;
		}
        if (grandchild >= N) {
            ArrayBinaryTree_resize(abt, grandchild+1);
		}
        
        // move right_child to its right subtree // might incur O(N) memory increase and build time
        ArrayBinaryTree_move_subtree(abt, right_child, move(right_child, DIR_RIGHT));
        // swap index and right_child
        ArrayBinaryTree_swap(abt, index, right_child);
        // swap grandchild and index
        ArrayBinaryTree_swap(abt, index, grandchild);
        // move granchild's right subtree to right_child's left subtree
        ArrayBinaryTree_move_subtree(abt, move(grandchild, DIR_RIGHT), move(right_child, DIR_LEFT));
        // move grandchild's left subtree to left_child's right subtree
        ArrayBinaryTree_move_subtree(abt, move(grandchild, DIR_LEFT), move(left_child, DIR_RIGHT));
        
    } else {
		return CL_FAILURE;
	}
    
    return CL_SUCCESS;
}
// check if node has a parent attribute and use a more efficient algorithm
size_t ArrayBinaryTree_leftmost(ArrayBinaryTree * abt, size_t index, Deque * path) {
	int destroy = (!path) ? 1 : 0;
	if (destroy) {
		path = Deque_new(2*ArrayBinaryTree_approx_height(abt));
	}
	
	size_t N = abt->size;
	while (index < N && abt->nodes[index] != NULL) {
		size_t * next_ = malloc(sizeof(size_t));
		*next_ = index;
		Deque_push_back(path, next_);
		index = move(index, DIR_LEFT);
	}
	
	size_t result = *(size_t*)Deque_peek_back(path);
	
    while (Deque_size(path)) {
        CL_FREE(Deque_pop_back(path));
    }
    Deque_del(path);
	
	return result;
}
// check if node has a parent attribute and use a more efficient algorithm
size_t ArrayBinaryTree_rightmost(ArrayBinaryTree * abt, size_t index, Deque * path) {
	if (!path) {
		path = Deque_new(2*ArrayBinaryTree_approx_height(abt));
	}
	
	size_t N = abt->size;
	while (index < N && abt->nodes[index] != NULL) {
		size_t * next_ = CL_MALLOC(sizeof(size_t));
		*next_ = index;
		Deque_push_back(path, next_);
		index = move(index, DIR_RIGHT);
	}
	
	size_t result = *(size_t*)Deque_peek_back(path);
	
    while (Deque_size(path)) {
        CL_FREE(Deque_pop_back(path));
    }
    Deque_del(path);

	return result;
}

/* // not called anywhere? Also, the output of this is a little weird. Probably can just replace it with the max_min_st tuple
// max_min_st must be freed by caller
static max_min_st * ArrayBinaryTree_extremal_paths_to_leaves(ArrayBinaryTree * abt, size_t index) {
	if (index >= abt->size || abt->nodes[index] == NULL) {
		return NULL;
	}
	
	max_min_st * left = ArrayBinaryTree_extremal_paths_to_leaves(abt, move(index, DIR_LEFT));
	max_min_st * right = ArrayBinaryTree_extremal_paths_to_leaves(abt, move(index, DIR_RIGHT));
	
	if (!left && !right) {
		return max_min_st_new(index, index);
	} else if (!right) {
		return left;
	} else if (!left) {
		return right;
	}
	
	max_min_st * out = max_min_st_new((left->max > right->max) ? left->max : right->max, (left->min < right->min) ? left->min : right->min);
	max_min_st_del(left);
	max_min_st_del(right);
	return out;
}
*/

static unsigned char ArrayBinaryTree_path_to_helper(ArrayBinaryTree * abt, size_t index, void * tvd) {
	if (!abt->compare(Node_get(abt->NA, abt->nodes[index], KEY), ((treevalue_deque*)tvd)->value)) {
        size_t * el = CL_MALLOC(sizeof(size_t));
        *el = index;
        Deque_push_back(((treevalue_deque*)tvd)->deq, el);
        return TRAVERSE_STOP;
    }
	return TRAVERSE_GO;
}

// TODO: need to fix when ArrayBinaryTree_traverse is available
Deque * ArrayBinaryTree_path_to(ArrayBinaryTree * abt, void * value) {
	Deque * st_out = Deque_new(ArrayBinaryTree_approx_height(abt));
	treevalue_deque helper_input = {value, st_out};
	
	//ArrayBinaryTree_traverse(abt, ArrayBinaryTree_path_to_helper, &helper_input);
	
	if (Deque_size(st_out)) {
		size_t * top = Deque_peek_back(st_out);
		size_t parent = move(*top, DIR_PARENT);
		while (parent < *top) {
			size_t * el = CL_MALLOC(sizeof(size_t));
			*el = parent;
			Deque_push_back(st_out, el);
			top = el;
			parent = move(*top, DIR_PARENT);
		}
	}
	
	Deque_reverse(st_out);
	return st_out;
}