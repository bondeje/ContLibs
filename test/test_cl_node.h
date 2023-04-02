//#include "../src/cl_core.h" // core includes cl_utils.h
#include <stdio.h>
#include <stdlib.h> // size_t, SIZE_MAX for size_t
#include <time.h>
#include <assert.h>
#include "cl_utils.h"
#include "cl_node.h"

// TODO: add asserts instead of printfs
int test_nodeattributes_new() {
    printf("Testing creating a new NodeAttributes...");
    unsigned int p = Node_flag(PARENT);
    unsigned int s = Node_flag(SIZE);
    unsigned int r = Node_flag(RIGHT);
    unsigned int c = Node_flag(COLOR);
    unsigned int ch = Node_flag(CHILD);

    printf("flags: size %u, parent %u, right %u, color %u, child %u\n", s, p, r, c, ch);

    NodeAttributes * NA = NodeAttributes_new(p | s | r | c | ch, 0);

    printf("NodeAttributes.n_attrs = %zu\n", NA->n_attrs);
    printf("NodeAttributes.size = %zu\n", NA->size);
    printf("i\tNodeAttributes.byte_map[i]\n");
    for (int i = 0; i < NA->n_attrs; i++) {
        printf("%d\t%u\n", i, NA->byte_map[i]);
    }
    printf("\n");
    printf("i\tNodeAttributes.attr_map[i]\n");
    for (int i = 0; i < NODE_N_ATTR; i++) {
        printf("%d\t%d\n", i, NA->attr_map[i]);
    }

    NodeAttributes_del(NA);

    printf("...PASS\n");
    return CL_SUCCESS;
}

/*
int test_node_new() {
    printf("Testing creating a new Node...");
    NodeAttributes * NA = NodeAttributes_new(Node_attr_flag(PARENT) | Node_attr_flag(SIZE) | Node_attr_flag(RIGHT) | Node_attr_flag(COLOR) | Node_attr_flag(CHILD), 0);

    Node * node = Node_new(NA, 0);

    printf("node created at %p\n", node);

    Node * child;

    printf("child node created at %p\n", child);

    size_t sz = 2;
    Node_set(NA, node, SIZE, &sz);
    printf("size sent %zu, size retrieved %zu\n", sz, Node_get(NA, node, SIZE));
    
    Node_set(NA, node, CHILD, &child);
    printf("child pointer sent %p, child pointer retrieved %p\n", child, Node_get(NA, node, CHILD));

    signed char clr = 1;
    Node_set(NA, node, COLOR, &clr);
    printf("color sent %hhi, color retrieved %hhi\n", clr, Node_get(NA, node, COLOR));

    Node_del(node);
    NodeAttributes_del(NA);

    printf("...PASS\n");
    return CL_SUCCESS;
}
*/

// hahaha, it works! 
int test_node_new() {
    printf("Testing creating a new Node...");
    char * test = "what";
    char * test_change = "this";
    NodeAttributes * NA = NodeAttributes_new(Node_flag(KEY) | Node_flag(PARENT) | Node_flag(SIZE) | Node_flag(RIGHT) | Node_flag(COLOR) | Node_flag(CHILD), 0);

    Node * node = Node_new(NA, 1, NODE_KEY, test);
    printf("new key arg: KEY, value: %p\n", (void*) test);
    if (Node_has(NA, KEY)) {
        const void * key = Node_get(NA, node, KEY);
        if (!key) {
            printf("in TEST key initialized to NULL: %p\n", key);
        } else {
            printf("in TEST key not initialized to NULL: %p = %s\n", key, (const char *)key);
        }
    }
    Node_set(NA, node, KEY, test_change);

    printf("node created at %p\n", node);

    Node * child; // not initialized! will be garbage/random

    printf("child node created at %p\n", child);

    size_t sz = 2;
    Node_set(NA, node, SIZE, sz);
    printf("size sent %zu, size retrieved %zu\n", sz, Node_get(NA, node, SIZE));
    
    Node_set(NA, node, CHILD, child);
    printf("child pointer sent %p, child pointer retrieved %p\n", child, Node_get(NA, node, CHILD));

    signed char clr = 1;
    Node_set(NA, node, COLOR, clr);
    printf("color sent %hhi, color retrieved %hhi\n", clr, Node_get(NA, node, COLOR));

    const char * retrieve = (const char *) Node_get(NA, node, KEY);
    printf("key sent: %s, key retrieved: %s\n", test, retrieve);

    Node_del(node);
    NodeAttributes_del(NA);

    printf("...PASS\n");
    return CL_SUCCESS;
}