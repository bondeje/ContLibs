#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include "cl_node.h"
#include "cl_linked_hash_set.h"

// TODO: eventually use macros to set the hash sizes so that user can replace hash function with a different output size
// TODO: incorporate a randomized hash seed for strings, "salting": see http://ocert.org/advisories/ocert-2011-003.html, referenced from https://docs.python.org/3.8/reference/datamodel.html#object.__hash__

#define PREV_INORDER LEFT
#define NEXT_INORDER RIGHT
#define NEXT_INHASH PARENT

#define REQUIRED_NODE_FLAGS (Node_flag(KEY) | Node_flag(LEFT) | Node_flag(RIGHT) | Node_flag(PARENT))

// manually construct static allocations of defaults. Reduce heap load
#define DEFAULT_SIZE (sizeof(Node_type(KEY)) + sizeof(Node_type(RIGHT)) + sizeof(Node_type(LEFT)) + sizeof(Node_type(PARENT)))
//static Node DEFAULT_NODE[DEFAULT_SIZE] = {'\0'}; // cannot do this because NodeAttributes_del(NA) expects NA->defaults to be NULL or heap-allocated
#define DEFAULT_NODE Node_new(NA, 4, Node_attr(KEY), NULL, Node_attr(RIGHT), NULL, Node_attr(LEFT), NULL, Node_attr(PARENT), NULL)

// set hash to NULL makes keys interpreted
struct LinkedHashSet {
    Node ** bins;
    Node * head_inorder;
    Node * tail_inorder;
    NodeAttributes * NA;
    size_t capacity; // allocation of hash_set, should be prime
    size_t size; // number of elements in hash_set
    float max_load_factor;
    int (*comp) (const void *, const void *);
    hash_t (*hash) (const void *, size_t);
};

struct LinkedHashSetIterator {
    NodeAttributes * NA;
    Node * node;
    const void * next_key;
    bool reversed;
    enum iterator_status stop;
};

/* INTERNAL SETTINGS */
// move to hash_utils
hash_t cstr_hash(const void * key, size_t bin_size) {
    unsigned long hash = 5381;
    int c;
    unsigned char * str = (unsigned char *) key;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash % bin_size;
}

hash_t address_hash(const void * val, size_t bin_size) {
    return ((char*)val - (char*)0) % bin_size; // subtracting (void*)0 so that it at least *looks* like I'm dealing with a number
}

int cstr_comp(const void * a, const void * b) {
    return strcmp((char *) a, (char *) b);
}

int address_comp(const void * a, const void * b) {
    if (a > b) {
        return 1;
    } else if (a < b) {
        return -1;
    }
    return 0;
}

size_t LinkedHashSet_size(LinkedHashSet * hash_set) {
    return hash_set->size;
}
size_t LinkedHashSet_capacity(LinkedHashSet * hash_set) {
    return hash_set->capacity;
}

int LinkedHashSet_resize(LinkedHashSet * hash_set, size_t capacity) {
    int result = CL_SUCCESS;
    Node * node = NULL, * last_node = NULL;

    // clear the bins and next_inhash on all nodes in the bins
    //printf("\nclearing bins");
    for (size_t i = 0; i < hash_set->capacity; i++) {
        node = hash_set->bins[i];
        while (node) {
            //printf("\nclearing node at bin %zu with pointer %p", i, (void*)node);
            last_node = node;
            node = Node_get(hash_set->NA, node, NEXT_INHASH);
            Node_set(hash_set->NA, last_node, NEXT_INHASH, NULL);
        }
        hash_set->bins[i] = NULL;
    }
    //printf("\nbins cleared");
    
    Node ** new_bins = (Node **) CL_REALLOC(hash_set->bins, sizeof(Node *) * capacity);
    
    if (!new_bins) {
        result = CL_FAILURE;
    } else {
        //printf("\nmemory allocated");

        hash_set->bins = new_bins;
        // clear the new bins
        for (size_t i = hash_set->capacity; i < capacity; i++) {
            hash_set->bins[i] = NULL;
        }

        // set new capacity
        hash_set->capacity = capacity;
    }

    // re-hash the keys back into the bins, proceeding in order
    node = hash_set->head_inorder;
    while (node) {
        hash_t bin = hash_set->hash(Node_get(hash_set->NA, node, KEY), hash_set->capacity);
        last_node = hash_set->bins[bin];
        Node_set(hash_set->NA, node, NEXT_INHASH, last_node);
        hash_set->bins[bin] = node;
        node = Node_get(hash_set->NA, node, NEXT_INORDER);
    }
    //printf("\nnodes reset...returning");
    return result;
}

LinkedHashSet * LinkedHashSet_new(hash_t (*hash) (const void *, size_t), int (*comp) (const void *, const void *), size_t capacity, float max_load_factor, unsigned int flags, int narg_pairs, ...) {
    LinkedHashSet * hash_set = (LinkedHashSet *) CL_MALLOC(sizeof(LinkedHashSet));
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

    if (!NA->defaults) { // if default fails, not necessarily a problem
        NA->defaults = DEFAULT_NODE;
    }

    LinkedHashSet_init(hash_set, hash, comp, capacity, max_load_factor, NA);    
    
    return hash_set;
}

void LinkedHashSet_init(LinkedHashSet * hash_set, hash_t (*hash) (const void *, size_t), int (*comp) (const void *, const void *), size_t capacity, float max_load_factor, NodeAttributes * NA) {
    hash_set->NA = NA;
    hash_set->head_inorder = NULL;
    hash_set->tail_inorder = NULL;
    hash_set->capacity = capacity;
    hash_set->size = 0;
    hash_set->max_load_factor = max_load_factor;
    hash_set->comp = comp;
    hash_set->hash = hash;

    for (size_t i = 0; i < capacity; i++) {
        hash_set->bins[i] = NULL;
    }
}
void LinkedHashSet_del(LinkedHashSet * hash_set) {
    while (hash_set->tail_inorder) {
        Node * node = hash_set->tail_inorder;
        hash_set->tail_inorder = Node_get(hash_set->NA, node, PREV_INORDER);
        if (hash_set->tail_inorder) {
            Node_set(hash_set->NA, hash_set->tail_inorder, NEXT_INORDER, NULL);
        }
        Node_del(node);
        hash_set->size--;
    }
    NodeAttributes_del(hash_set->NA);
    hash_set->NA = NULL;
    CL_FREE(hash_set->bins);
    hash_set->bins = NULL;
    CL_FREE(hash_set);
}

Node * LinkedHashSet_get_node(LinkedHashSet * hash_set, void * key) {
    Node * node = hash_set->bins[hash_set->hash(key, hash_set->capacity)];
    while (node && hash_set->comp(Node_get(hash_set->NA, node, KEY), key)) {
        node = Node_get(hash_set->NA, node, NEXT_INHASH);
    }
    return node;
}

int LinkedHashSet_add(LinkedHashSet * hash_set, void * key) {
    Node * node = LinkedHashSet_get_node(hash_set, key);
    // if node is found, overwrite the value
    if (node) { // do nothing if key already found
        return CL_SUCCESS;
    }

    // create new node and assign it to bins and linked list
    hash_t bin = hash_set->hash(key, hash_set->capacity);
    node = Node_new(hash_set->NA, 0);
    if (!node) {
        return CL_FAILURE;
    }
    Node_set(hash_set->NA, node, KEY, key);
    if (!hash_set->size) {
        hash_set->head_inorder = node;
        hash_set->tail_inorder = node;
    } else {
        Node_set(hash_set->NA, node, NEXT_INHASH, hash_set->bins[bin]);
        Node_set(hash_set->NA, node, PREV_INORDER, hash_set->tail_inorder);
        Node_set(hash_set->NA, hash_set->tail_inorder, NEXT_INORDER, node);
        hash_set->tail_inorder = node;
    }
    hash_set->bins[bin] = node;

    hash_set->size++;

    if (((float)hash_set->size) / hash_set->capacity > hash_set->max_load_factor) {
        LinkedHashSet_resize(hash_set, next_prime(hash_set->capacity*2));
    }

    return CL_SUCCESS;
}

bool LinkedHashSet_contains(LinkedHashSet * hash_set, void * key) {
    return LinkedHashSet_get_node(hash_set, key) != NULL;
}

// remove and return the node identified by the key in hash_set. Returns NULL if key is not found
static Node * LinkedHashSet_pop_(LinkedHashSet * hash_set, void * key) {
    //printf("\nin LinkedHashSet_pop_");
    Node * last_node = NULL, * node, * next_node = NULL;
    hash_t bin = hash_set->hash(key, hash_set->capacity);
    node = hash_set->bins[bin];
    while (node && hash_set->comp(Node_get(hash_set->NA, node, KEY), key)) {
        last_node = node;
        node = Node_get(hash_set->NA, node, NEXT_INHASH);
    }

    if (!node) {
        return NULL; // failure to remove that which is not present
    }

    //printf("\nafter finding node\n\tlast_node: %p\n\tnode: %p", (void*)last_node, (void*)node);

    // node is now the node to be removed
    // last_node is the node preceding the node to be removed in the bin
    if (last_node) {
        //printf("\n\tlast_node->next_inhash: %p", (void*)last_node->next_inhash);
        Node_set(hash_set->NA, last_node, NEXT_INHASH, Node_get(hash_set->NA, node, NEXT_INHASH));
    } else { // node to be removed is stored in the bin
        hash_set->bins[bin] = Node_get(hash_set->NA, node, NEXT_INHASH);
    }
    //printf("\n\tnode->next_inhash: %p", (void*)node->next_inhash);

    // node is now removed from hash_set->bins, need to remove from linked list
    last_node = Node_get(hash_set->NA, node, PREV_INORDER);
    Node_set(hash_set->NA, node, PREV_INORDER, NULL);
    next_node = Node_get(hash_set->NA, node, NEXT_INORDER);
    Node_set(hash_set->NA, node, NEXT_INORDER, NULL);
    //printf("\nafter removing node from linked list\n\tlast_node: %p\n\tnext_node: %p", (void*)last_node, (void*)next_node);
    //printf("\n\tnode->prev_inorder: %p\n\tnode->next_inorder: %p", (void*)node->prev_inorder, (void*)node->next_inorder);
    if (last_node) {
        Node_set(hash_set->NA, last_node, NEXT_INORDER, next_node);
    }
    if (next_node) {
        Node_set(hash_set->NA, next_node, PREV_INORDER, last_node);
    }

    //printf("\n\thash_set->head_inorder: %p\n\thash_set->tail_inorder: %p", (void*)hash_set->head_inorder, (void*)hash_set->tail_inorder);

    // reset ends of linked list if necessary

    if (hash_set->head_inorder == node) {
        hash_set->head_inorder = next_node;
    }
    if (hash_set->tail_inorder == node) {
        hash_set->tail_inorder = last_node;
    }

    hash_set->size--;

    // TODO: check load factor and resize if necessary

    return node;
}

// removes and destroys the node identified by key. returns 0 if successful (key is found)
int LinkedHashSet_remove(LinkedHashSet * hash_set, void * key) {
    Node * to_rem = LinkedHashSet_pop_(hash_set, key);
    if (!to_rem) {
        return CL_FAILURE;
    }
    Node_del(to_rem);
    return CL_SUCCESS;
}

// ITERATORS:

LinkedHashSetIterator * LinkedHashSetIterator_new(LinkedHashSet * hash_set, bool reversed) {
    LinkedHashSetIterator * key_iter = (LinkedHashSetIterator *) CL_MALLOC(sizeof(LinkedHashSetIterator));
    if (!key_iter) {
        return NULL;
    }
    LinkedHashSetIterator_init(key_iter, hash_set, reversed);
    return key_iter;
}
void LinkedHashSetIterator_init(LinkedHashSetIterator * key_iter, LinkedHashSet * hash_set, bool reversed) {
    key_iter->next_key = NULL;
    if (reversed) {
        key_iter->node = hash_set->tail_inorder;
    } else {
        key_iter->node = hash_set->head_inorder;
    }
    key_iter->NA = hash_set->NA;
    key_iter->reversed = reversed;
    key_iter->stop = ITERATOR_GO;
}
void LinkedHashSetIterator_del(LinkedHashSetIterator * key_iter) {
    key_iter->next_key = NULL;
    key_iter->node = NULL;
    key_iter->NA = NULL;
    CL_FREE(key_iter);
}

LinkedHashSetIterator * LinkedHashSetIterator_iter(LinkedHashSet * hash_set, bool reversed) {
    return LinkedHashSetIterator_new(hash_set, reversed);
}
const void * LinkedHashSetIterator_next(LinkedHashSetIterator * key_iter) {
    if (!key_iter) {
        return NULL;
    }
    if (!key_iter->node) {
        key_iter->stop = ITERATOR_STOP;
        return NULL;
    }
    key_iter->next_key = Node_get(key_iter->NA, key_iter->node, KEY);
    if (key_iter->reversed) {
        key_iter->node = Node_get(key_iter->NA, key_iter->node, PREV_INORDER);
    } else {
        key_iter->node = Node_get(key_iter->NA, key_iter->node, NEXT_INORDER);
    }
    return key_iter->next_key;
}
enum iterator_status LinkedHashSetIterator_stop(LinkedHashSetIterator * key_iter) {
    if (!key_iter) {
        return ITERATOR_STOP;
    }
    if (key_iter->stop == ITERATOR_STOP) {
        LinkedHashSetIterator_del(key_iter);
        return ITERATOR_STOP;
    }
    return key_iter->stop;
}
