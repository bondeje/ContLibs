#include <stddef.h>

#ifndef CL_ARRAY_BINARY_TREE_H
#define CL_ARRAY_BINARY_TREE_H

typedef struct ArrayBinaryTree ArrayBinaryTree;

enum abt_status {
	ABT_SUCCESS = 0,
	ABT_FAILURE = -1,
	ABT_ALLOC_FAILURE = -2,
	ABT_REALLOC_FAILURE = -3,
	ABT_INDEX_OUT_OF_BOUNDS = -10,
};

#endif // CL_ARRAY_BINARY_TREE_H