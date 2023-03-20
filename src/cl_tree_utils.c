#include <stddef.h>
#include "cl_tree_utils.h"

size_t array_move_index(size_t index, int dir) {
	if (!dir) { // dir == DIR_PARENT
		return ((index - 1) >> 1);
	}
	return (index << 1) + (1-(dir - 1)/2); // addition at the end maps (1, -1) to (1, 2)
}