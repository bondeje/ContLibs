#ifndef CL_TREE_UTILS_H
#define CL_TREE_UTILS_H

#define DIR_LEFT 1
#define DIR_PARENT 0
#define DIR_RIGHT -1 // there's a reason for doing this...since index -1 is the end of the minus array, this might work with the CHILDREN attribute 

// DEFAULT storages are up to each implementation
#define STORAGE_TYPE char
#define STORAGE_LINKED 'L'
#define STORAGE_ARRAY 'A'

#define TREE_FLAG_TYPE unsigned int

#define TREE_ITERATE_ORDER_MASK			0x03
#define TREE_ITERATE_INORDER			0x00
#define TREE_ITERATE_PREORDER			0x01
#define TREE_ITERATE_POSTORDER			0x02
#define TREE_ITERATE_LEVELORDER			0x03
#define TREE_ITERATE_REVERSE			0x04

#define TRAVERSE_STOP				0x00
#define TRAVERSE_GO					0x01

#define TREE_MOD_UNIQUE				0x01
#define TREE_MOD_UPDATE				0x02
#define TREE_MOD_ORDER_MASK			0x0C
#define TREE_MOD_FIRST_INORDER		0x04
#define TREE_MOD_LAST_INORDER		0x08
#define TREE_MOD_FIRST_LEVELORDER	0x00 // default...if not order is set, then TREE_MOD_ORDER_MASK is 0

size_t array_move_index(size_t index, int dir);

#endif // CL_TREE_UTILS