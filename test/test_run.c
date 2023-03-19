#include <stdio.h>
#include "test_cl_utils.h"

#ifndef NCL_NODE
#include "test_cl_node.h"
#endif // NCL_NODE


#ifndef NCL_CONTAINER
#include "test_cl_container.h"
#endif // NCL_CONTAINER

#ifndef NCL_DBLLINKEDLIST
#include "test_cl_dbllinkedlist.h"
#endif // NCL_DBLLINKEDLIST

int main() {

    // TODO: include test summary statistics

#ifndef NCL_UTILS
    printf("testing cl_utils.h\n");
	test_swaps();
	test_reverse();

#ifdef INCLUDE_TEST_DEBUG
	test_debug();
#endif // INCLUDE_TEST_DEBUG

    printf("\n");
#endif // NCL_UTILS

#ifndef NCL_CONTAINER
    printf("testing cl_containers.h\n");
    //test_container_new();
    printf("\n");
#endif // NCL_CONTAINER

#ifndef NCL_NODE
	printf("testing cl_node.h\n");

	test_nodeattributes_new();
	test_node_new();

	printf("\n");
#endif // NCL_NODE

#ifndef NCL_DBLLINKEDLIST
	printf("testing cl_dbllinkedlist.h\n");

	test_DblLinkedList_new();

	printf("\n");
#endif // NCL_DBLLINKEDLIST

	return CL_SUCCESS;
}