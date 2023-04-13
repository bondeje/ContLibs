#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
//#include "cl_node.h"
#include "cl_iterators.h"
#include "cl_dbl_linked_hash_table.h"

// TODO: eventually use macros to set the hash sizes so that user can replace hash function with a different output size
// TODO: incorporate a randomized hash seed for strings, "salting": see http://ocert.org/advisories/ocert-2011-003.html, referenced from https://docs.python.org/3.8/reference/datamodel.html#object.__hash__

// in order to "inherit" linked list for ordering, NEXT_INORDER must be an alias for NEXT while NEXT_INHASH cannot
#define PREV_INORDER PREV
#define NEXT_INORDER NEXT
#define NEXT_INHASH RIGHT

#define REQUIRED_NODE_FLAGS (Node_flag(KEY) | Node_flag(VALUE) | Node_flag(NEXT_INHASH) | Node_flag(NEXT_INORDER) | Node_flag(PREV_INORDER))// | Node_flag(LEFT))
// TODO: replace with a default NodeAttributes, DefaultNode at file scope
#define DEFAULT_NODE Node_new(NA, 4, Node_attr(VALUE), NULL, Node_attr(KEY), NULL, Node_attr(NEXT_INORDER), NULL, Node_attr(PREV_INORDER), NULL, Node_attr(NEXT_INHASH), NULL)//, Node_attr(PREV_INORDER), NULL

// set hash to NULL makes keys interpreted
/*
struct DblLinkedHashTable {
    NodeAttributes * NA;
    Node * head;
    Node * tail;
    Node ** bins; // the bins for the table themselves cannot be a LinkedList because the linkages are RIGHT and not NEXT
    size_t size; // number of elements in hash_table
    size_t capacity; // allocation of hash_table, should be prime
    float max_load_factor;
    int (*comp) (const void *, const void *);
    hash_t (*hash) (const void *, size_t);
};
*/

size_t DblLinkedHashTable_size(DblLinkedHashTable * hash_table) {
    return LinkedHashTable_size(hash_table);
}
size_t DblLinkedHashTable_capacity(DblLinkedHashTable * hash_table) {
    return LinkedHashTable_capacity(hash_table);
}

int DblLinkedHashTable_resize(DblLinkedHashTable * hash_table, size_t capacity) {
    return LinkedHashTable_resize(hash_table, capacity);
}

DblLinkedHashTable * DblLinkedHashTable_new(hash_t (*hash) (const void *, size_t), int (*comp) (const void *, const void *), size_t capacity, float max_load_factor, unsigned int flags, int narg_pairs, ...) {
    DblLinkedHashTable * hash_table = (DblLinkedHashTable *) CL_MALLOC(sizeof(DblLinkedHashTable));
    if (!hash_table) {
        return NULL;
    }

    if (!capacity) {
        capacity = LINKED_HASH_TABLE_DEFAULT_CAPACITY;
    }

    //hash_table->bins = (DoubleLinkedHashNode **) CL_MALLOC(sizeof(DoubleLinkedHashNode*) * capacity);
    hash_table->bins = (Node **) CL_MALLOC(sizeof(Node*) * capacity);
    if (!hash_table->bins) {
        CL_FREE(hash_table);
        return NULL;
    }

    if (!hash) { // if no hash is provided, default to hashing on the address and comparing addresses
        hash = address_hash;
        comp = address_comp;
    }

    if (max_load_factor <= 0) {
        max_load_factor = LINKED_HASH_TABLE_LOAD_FACTOR;
    }

    // TODO: encapsulate the following five lines for the case of flags > 0. For flags == 0, use a DEFAULT_NODE_ATTRIBUTES. See e.g. cl_array_binary_tree.c
    flags |= REQUIRED_NODE_FLAGS; // must have these flag minimum
    va_list args;
    va_start(args, narg_pairs);
    NodeAttributes * NA = vNodeAttributes_new(flags, narg_pairs, args);
    va_end(args);

    if (!NA) {
        CL_FREE(hash_table->bins);
        CL_FREE(hash_table);
        return NULL;
    }

    if (!narg_pairs) { // if default fails, not necessarily a problem
        NodeAttributes_set_default_node(NA, DEFAULT_NODE);
        NA->default_alloc = true;
    }

    DblLinkedHashTable_init(hash_table, hash, comp, capacity, max_load_factor, NA);
    
    
    return hash_table;
}

void DblLinkedHashTable_init(DblLinkedHashTable * hash_table, hash_t (*hash) (const void *, size_t), int (*comp) (const void *, const void *), size_t capacity, float max_load_factor, NodeAttributes * NA) {
    LinkedHashTable_init(hash_table, hash, comp, capacity, max_load_factor, NA);
}
void DblLinkedHashTable_del(DblLinkedHashTable * hash_table) {
    LinkedHashTable_del(hash_table);
}

Node * DblLinkedHashTable_get_node(DblLinkedHashTable * hash_table, void * key) {
    return LinkedHashTable_get_node(hash_table, key);
}

int DblLinkedHashTable_set(DblLinkedHashTable * hash_table, void * key, void * value) {
    Node * node = DblLinkedHashTable_get_node(hash_table, key);
    // if node is found, overwrite the value
    if (node) {
        Node_set(hash_table->NA, node, VALUE, value);
        return CL_SUCCESS;
    }

    // create new node and assign it to bins and linked list
    hash_t bin = hash_table->hash(key, hash_table->capacity);
    // BUG: this next line should be sufficient rather than default init and set later, but there appears to be a bug in Node_new
    node = Node_new(hash_table->NA, 4, Node_attr(KEY), key, Node_attr(VALUE), value, Node_attr(NEXT_INHASH), hash_table->bins[bin], Node_attr(PREV_INORDER), hash_table->tail);
    
    if (!node) {
        return CL_MALLOC_FAILURE;
    }
    if (!hash_table->tail) {
        hash_table->head = node;
    } else {
        Node_set(hash_table->NA, hash_table->tail, NEXT_INORDER, node);
    }
    hash_table->tail = node;
    hash_table->bins[bin] = node;

    hash_table->size++;

    if (((float)hash_table->size) / hash_table->capacity > hash_table->max_load_factor) {
        DblLinkedHashTable_resize(hash_table, next_prime(hash_table->capacity*2));
    }

    return CL_SUCCESS;
}

void * DblLinkedHashTable_get(DblLinkedHashTable * hash_table, void * key) {
    return LinkedHashTable_get(hash_table, key);
}

bool DblLinkedHashTable_contains(DblLinkedHashTable * hash_table, void * key) {
    return LinkedHashTable_contains(hash_table, key);
}

// remove and return the node identified by the key in hash_table. Returns NULL if key is not found
static Node * DblLinkedHashTable_pop_(DblLinkedHashTable * hash_table, void * key) {
    //printf("\nin DblLinkedHashTable_pop_");
    Node * last_node = NULL, * node, * next_node = NULL, * to_del = NULL;
    hash_t bin = hash_table->hash(key, hash_table->capacity);
    node = hash_table->bins[bin];
    while (node && hash_table->comp(Node_get(hash_table->NA, node, KEY), key)) {
        last_node = node;
        node = Node_get(hash_table->NA, node, NEXT_INHASH);
    }

    if (!node) {
        return NULL; // failure to remove that which is not present
    }

    next_node = Node_get(hash_table->NA, node, NEXT_INHASH);

    // remove from linked list in bin
    // TODO: when I make an unordered hash_table/set, need to make a function to pop from linked list in bin and replace the following code with it
    if (last_node) {
        Node_set(hash_table->NA, last_node, NEXT_INHASH, next_node);
    } else {
        hash_table->bins[bin] = next_node;
    }
    Node_set(hash_table->NA, node, NEXT_INHASH, NULL);

    // remove from linked list for ordering
    // so long as NEXT_INORDER is an alias for NEXT, we can actually replace this
    // with initialization of a LinkedList and pop the node for code re-use
    // this requires a function in the LinkedList module: LinkedList_pop_node(LinkedList * ll, Node * to_pop)
    to_del = hash_table->head;
    last_node = NULL;
    while (to_del && to_del != node) { 
        last_node = to_del;
        to_del = Node_get(hash_table->NA, to_del, NEXT_INORDER);
    }
    next_node = Node_get(hash_table->NA, node, NEXT_INORDER);
    if (last_node) {
        Node_set(hash_table->NA, last_node, NEXT_INORDER, next_node);
    } else {
        hash_table->head = next_node;
    }
    if (!next_node) {
        hash_table->tail = last_node;
    } else {
        Node_set(hash_table->NA, next_node, PREV_INORDER, last_node);
    }
    hash_table->size--;
    // TODO: check load factor and resize if necessary
    return node;
}

// removes and destroys the node identified by key. returns the value at the node. Warning, NULL is a perfectly acceptable value to store
// so to verify it is removed, you have to either check contains or use DblLinkedHashTable_remove and check for failure
void * DblLinkedHashTable_pop(DblLinkedHashTable * hash_table, void * key) {
    Node * to_rem = DblLinkedHashTable_pop_(hash_table, key);
    if (!to_rem) {
        return NULL;
    }
    void * val = Node_get(hash_table->NA, to_rem, VALUE);
    Node_del(to_rem);
    return val;
}

// removes and destroys the node identified by key. returns 0 if successful (key is found)
int DblLinkedHashTable_remove(DblLinkedHashTable * hash_table, void * key) {
    Node * to_rem = DblLinkedHashTable_pop_(hash_table, key);
    if (!to_rem) {
        return CL_FAILURE;
    }
    Node_del(to_rem);
    return CL_SUCCESS;
}

// ITERATORS:

DblLinkedHashTableKeyIterator * DblLinkedHashTable_keys(DblLinkedHashTable * hash_table) {
    return LinkedHashTableKeyIterator_new(hash_table);
}
DblLinkedHashTableValueIterator * DblLinkedHashTable_values(DblLinkedHashTable * hash_table) {
    return LinkedHashTableValueIterator_new(hash_table);
}
DblLinkedHashTableItemIterator * DblLinkedHashTable_items(DblLinkedHashTable * hash_table) {
    return LinkedHashTableItemIterator_new(hash_table);
}

DblLinkedHashTableKeyIterator * DblLinkedHashTableKeyIterator_new(DblLinkedHashTable * hash_table) {
    return LinkedHashTableKeyIterator_new(hash_table);
}
DblLinkedHashTableValueIterator * DblLinkedHashTableValueIterator_new(DblLinkedHashTable * hash_table) {
    return LinkedHashTableValueIterator_new(hash_table);
}
DblLinkedHashTableItemIterator * DblLinkedHashTableItemIterator_new(DblLinkedHashTable * hash_table) {
    return LinkedHashTableItemIterator_new(hash_table);
}
void DblLinkedHashTableKeyIterator_init(DblLinkedHashTableKeyIterator * key_iter, DblLinkedHashTable * hash_table) {
    LinkedHashTableKeyIterator_init(key_iter, hash_table);
}
void DblLinkedHashTableValueIterator_init(DblLinkedHashTableValueIterator * value_iter, DblLinkedHashTable * hash_table) {
    LinkedHashTableValueIterator_init(value_iter, hash_table);
}
void DblLinkedHashTableItemIterator_init(DblLinkedHashTableItemIterator * item_iter, DblLinkedHashTable * hash_table) {
    LinkedHashTableItemIterator_init(item_iter, hash_table);
}
void DblLinkedHashTableKeyIterator_del(DblLinkedHashTableKeyIterator * key_iter) {
    LinkedHashTableKeyIterator_del(key_iter);
}
void DblLinkedHashTableValueIterator_del(DblLinkedHashTableValueIterator * value_iter) {
    LinkedHashTableValueIterator_del(value_iter);
}
void DblLinkedHashTableItemIterator_del(DblLinkedHashTableItemIterator * item_iter) {
    LinkedHashTableItemIterator_del(item_iter);
}

const void * DblLinkedHashTableKeyIterator_next(DblLinkedHashTableKeyIterator * key_iter) {
    return LinkedHashTableKeyIterator_next(key_iter);
}
void * DblLinkedHashTableValueIterator_next(DblLinkedHashTableValueIterator * value_iter) {
    return LinkedHashTableKeyIterator_next(value_iter);
}
DictItem * DblLinkedHashTableItemIterator_next(DblLinkedHashTableItemIterator * item_iter)  {
    return LinkedHashTableItemIterator_next(item_iter);
}
enum iterator_status DblLinkedHashTableKeyIterator_stop(DblLinkedHashTableKeyIterator * key_iter) {
    return LinkedHashTableKeyIterator_stop(key_iter);
}
enum iterator_status DblLinkedHashTableValueIterator_stop(DblLinkedHashTableValueIterator * value_iter) {
    return LinkedHashTableValueIterator_stop(value_iter);
}
enum iterator_status DblLinkedHashTableItemIterator_stop(DblLinkedHashTableItemIterator * item_iter) {
    return LinkedHashTableItemIterator_stop(item_iter);
}
