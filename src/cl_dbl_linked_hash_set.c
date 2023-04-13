#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include "cl_dbl_linked_hash_set.h"

// TODO: eventually use macros to set the hash sizes so that user can replace hash function with a different output size
// TODO: incorporate a randomized hash seed for strings, "salting": see http://ocert.org/advisories/ocert-2011-003.html, referenced from https://docs.python.org/3.8/reference/datamodel.html#object.__hash__

#define PREV_INORDER PREV
#define NEXT_INORDER NEXT
#define NEXT_INHASH RIGHT

#define REQUIRED_NODE_FLAGS (Node_flag(KEY) | Node_flag(NEXT_INORDER) | Node_flag(NEXT_INHASH))

// manually construct static allocations of defaults. Reduce heap load
#define DEFAULT_SIZE (sizeof(Node_type(KEY)) + sizeof(Node_type(NEXT_INORDER)) + sizeof(Node_type(NEXT_INHASH)))
//static Node DEFAULT_NODE[DEFAULT_SIZE] = {'\0'}; // cannot do this because NodeAttributes_del(NA) expects NA->defaults to be NULL or heap-allocated
#define DEFAULT_NODE Node_new(NA, 3, Node_attr(KEY), NULL, Node_attr(NEXT_INORDER), NULL, Node_attr(NEXT_INHASH), NULL)

size_t DblLinkedHashSet_size(DblLinkedHashSet * hash_set) {
    return LinkedHashSet_size(hash_set);
}
size_t DblLinkedHashSet_capacity(DblLinkedHashSet * hash_set) {
    return LinkedHashSet_capacity(hash_set);
}

int DblLinkedHashSet_resize(DblLinkedHashSet * hash_set, size_t capacity) {
    return LinkedHashSet_resize(hash_set, capacity);
}

DblLinkedHashSet * DblLinkedHashSet_new(hash_t (*hash) (const void *, size_t), int (*comp) (const void *, const void *), size_t capacity, float max_load_factor, unsigned int flags, int narg_pairs, ...) {
    DblLinkedHashSet * hash_set = (DblLinkedHashSet *) CL_MALLOC(sizeof(DblLinkedHashSet));
    if (!hash_set) {
        return NULL;
    }

    if (!capacity) {
        capacity = LINKED_HASH_SET_DEFAULT_CAPACITY;
    }

    //hash_set->bins = (DoubleLinkedHashNode **) CL_MALLOC(sizeof(DoubleLinkedHashNode*) * capacity);
    hash_set->bins = (Node **) CL_MALLOC(sizeof(Node*) * capacity);
    if (!hash_set->bins) {
        CL_FREE(hash_set);
        return NULL;
    }

    if (!hash) { // if no hash is provided, default to hashing on the address and comparing addresses
        hash = address_hash;
        comp = address_comp;
    }

    if (max_load_factor <= 0) {
        max_load_factor = LINKED_HASH_SET_LOAD_FACTOR;
    }

    // TODO: encapsulate the following five lines for the case of flags > 0. For flags == 0, use a DEFAULT_NODE_ATTRIBUTES. See e.g. cl_array_binary_tree.c
    flags |= REQUIRED_NODE_FLAGS; // must have these flag minimum
    va_list args;
    va_start(args, narg_pairs);
    NodeAttributes * NA = vNodeAttributes_new(flags, narg_pairs, args);
    va_end(args);

    if (!NA) {
        CL_FREE(hash_set->bins);
        CL_FREE(hash_set);
        return NULL;
    }

    if (!narg_pairs) { // if default fails, not necessarily a problem
        NodeAttributes_set_default_node(NA, DEFAULT_NODE);
        NA->default_alloc = true;
    }

    DblLinkedHashSet_init(hash_set, hash, comp, capacity, max_load_factor, NA);    
    
    return hash_set;
}

void DblLinkedHashSet_init(DblLinkedHashSet * hash_set, hash_t (*hash) (const void *, size_t), int (*comp) (const void *, const void *), size_t capacity, float max_load_factor, NodeAttributes * NA) {
    LinkedHashSet_init(hash_set, hash, comp, capacity, max_load_factor, NA);
}
void DblLinkedHashSet_del(DblLinkedHashSet * hash_set) {
    LinkedHashSet_del(hash_set);
}

Node * DblLinkedHashSet_get_node(DblLinkedHashSet * hash_set, void * key) {
    return LinkedHashSet_get_node(hash_set, key);
}

int DblLinkedHashSet_add(DblLinkedHashSet * hash_set, void * key) {
    Node * node = DblLinkedHashSet_get_node(hash_set, key);
    // if node is found, overwrite the value
    if (node) { // do nothing if key already found
        return CL_SUCCESS;
    }

    // create new node and assign it to bins and linked list
    hash_t bin = hash_set->hash(key, hash_set->capacity);
    node = Node_new(hash_set->NA, 3, Node_attr(KEY), key, Node_attr(NEXT_INHASH), hash_set->bins[bin], Node_attr(PREV_INORDER), hash_set->tail);
    if (!node) {
        return CL_MALLOC_FAILURE;
    }
    if (!hash_set->tail) {
        hash_set->head = node;
    } else {
        Node_set(hash_set->NA, hash_set->tail, NEXT_INORDER, node);
    }
    hash_set->tail = node;
    hash_set->bins[bin] = node;

    hash_set->size++;

    if (((float)hash_set->size) / hash_set->capacity > hash_set->max_load_factor) {
        DblLinkedHashSet_resize(hash_set, next_prime(hash_set->capacity*2));
    }

    return CL_SUCCESS;
}

bool DblLinkedHashSet_contains(DblLinkedHashSet * hash_set, void * key) {
    return LinkedHashSet_contains(hash_set, key);
}

// remove and return the node identified by the key in hash_set. Returns NULL if key is not found
static Node * DblLinkedHashSet_pop_(DblLinkedHashSet * hash_set, void * key) {
    //printf("\nin DblLinkedHashSet_pop_");
    Node * last_node = NULL, * node, * next_node = NULL, * to_del = NULL;
    hash_t bin = hash_set->hash(key, hash_set->capacity);
    node = hash_set->bins[bin];
    while (node && hash_set->comp(Node_get(hash_set->NA, node, KEY), key)) {
        last_node = node;
        node = Node_get(hash_set->NA, node, NEXT_INHASH);
    }

    if (!node) {
        return NULL; // failure to remove that which is not present
    }

    next_node = Node_get(hash_set->NA, node, NEXT_INHASH);

    // remove from linked list in bin
    // TODO: when I make an unordered hash_table/set, need to make a function to pop from linked list in bin and replace the following code with it
    if (last_node) {
        //printf("\n\tlast_node->next_inhash: %p", (void*)last_node->next_inhash);
        Node_set(hash_set->NA, last_node, NEXT_INHASH, next_node);
    } else { // node to be removed is stored in the bin
        hash_set->bins[bin] = next_node;
    }
    Node_set(hash_set->NA, node, NEXT_INHASH, NULL);

    // remove from linked list for ordering
    // TODO: so long as NEXT_INORDER is an alias for NEXT, we can actually replace this
    // with initialization of a LinkedList and pop the node for code re-use
    // this requires a function in the LinkedList module: LinkedList_pop_node(LinkedList * ll, Node * to_pop)
    to_del = hash_set->head;
    last_node = NULL;
    while (to_del && to_del != node) { 
        last_node = to_del;
        to_del = Node_get(hash_set->NA, to_del, NEXT_INORDER);
    }
    next_node = Node_get(hash_set->NA, node, NEXT_INORDER);
    if (last_node) {
        Node_set(hash_set->NA, last_node, NEXT_INORDER, next_node);
    } else {
        hash_set->head = next_node;
    }
    if (!next_node) {
        hash_set->tail = last_node;
    } else {
        Node_set(hash_set->NA, next_node, PREV_INORDER, last_node);
    }
    hash_set->size--;

    // TODO: check load factor and resize if necessary

    return node;
}

// removes and destroys the node identified by key. returns 0 if successful (key is found)
int DblLinkedHashSet_remove(DblLinkedHashSet * hash_set, void * key) {
    Node * to_rem = DblLinkedHashSet_pop_(hash_set, key);
    if (!to_rem) {
        return CL_FAILURE;
    }
    Node_del(to_rem);
    return CL_SUCCESS;
}

// ITERATORS:

DblLinkedHashSetIterator * DblLinkedHashSetIterator_new(DblLinkedHashSet * hash_set) {
    return LinkedHashSetIterator_new(hash_set);
}
void DblLinkedHashSetIterator_init(DblLinkedHashSetIterator * key_iter, DblLinkedHashSet * hash_set) {
    LinkedhHashSetIterator_init(key_iter, hash_set);
}
void DblLinkedHashSetIterator_del(DblLinkedHashSetIterator * key_iter) {
    LinkedHashSetIterator_del(key_iter);
}

const void * DblLinkedHashSetIterator_next(DblLinkedHashSetIterator * key_iter) {
    return LinkedHashSetIterator_next(key_iter);
}
enum iterator_status DblLinkedHashSetIterator_stop(DblLinkedHashSetIterator * key_iter) {
    return LinkedHashSetIterator_stop(key_iter);
}
