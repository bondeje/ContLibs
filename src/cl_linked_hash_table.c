#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include "cl_node.h"
#include "cl_linked_hash_table.h"

// TODO: eventually use macros to set the hash sizes so that user can replace hash function with a different output size
// TODO: incorporate a randomized hash seed for strings, "salting": see http://ocert.org/advisories/ocert-2011-003.html, referenced from https://docs.python.org/3.8/reference/datamodel.html#object.__hash__

#define PREV_INORDER LEFT
#define NEXT_INORDER RIGHT
#define NEXT_INHASH PARENT

#define REQUIRED_NODE_FLAGS (Node_flag(KEY) | Node_flag(VALUE) | Node_flag(PARENT) | Node_flag(LEFT) | Node_flag(RIGHT))
// TODO: replace with a default NodeAttributes, DefaultNode at file scope
#define DEFAULT_NODE Node_new(NA, 5, Node_attr(VALUE), NULL, Node_attr(KEY), NULL, Node_attr(PREV_INORDER), NULL, Node_attr(NEXT_INORDER), NULL, Node_attr(NEXT_INHASH), NULL)

// set hash to NULL makes keys interpreted
struct LinkedHashTable {
    Node ** bins;
    Node * head_inorder;
    Node * tail_inorder;
    NodeAttributes * NA;
    size_t capacity; // allocation of hash_table, should be prime
    size_t size; // number of elements in hash_table
    float max_load_factor;
    int (*comp) (const void *, const void *);
    hash_t (*hash) (const void *, size_t);
};

struct LinkedHashTableKeyIterator {
    NodeAttributes * NA;
    Node * node;
    const void * next_key;
    bool reversed;
    enum iterator_status stop;
};

struct LinkedHashTableValueIterator {
    NodeAttributes * NA;
    Node * node;
    void * next_value;
    bool reversed;
    enum iterator_status stop;
};

struct LinkedHashTableItemIterator {
    NodeAttributes * NA;
    Node * node;
    DictItem * next_item;
    bool reversed;
    enum iterator_status stop;
};

/* INTERNAL SETTINGS */

DictItem * DictItem_new(void * key, void * value) {
    DictItem * di = (DictItem *) CL_MALLOC(sizeof(DictItem));
    if (!di) {
        return NULL;
    }
    DictItem_init(di, key, value);
    return di;
}
void DictItem_init(DictItem * di, void * key, void * value) {
    di->key = key;
    di->value = value;
}
// TODO: change out free for the selectable dynamic memory allocation macros
void DictItem_del(DictItem * di) {
    di->key = NULL;
    di->value = NULL;
    CL_FREE(di);
}

size_t LinkedHashTable_size(LinkedHashTable * hash_table) {
    return hash_table->size;
}
size_t LinkedHashTable_capacity(LinkedHashTable * hash_table) {
    return hash_table->capacity;
}

int LinkedHashTable_resize(LinkedHashTable * hash_table, size_t capacity) {
    int result = CL_SUCCESS;
    Node * node = NULL, * last_node = NULL;

    // clear the bins and next_inhash on all nodes in the bins
    //printf("\nclearing bins");
    for (size_t i = 0; i < hash_table->capacity; i++) {
        node = hash_table->bins[i];
        while (node) {
            //printf("\nclearing node at bin %zu with pointer %p", i, (void*)node);
            last_node = node;
            node = Node_get(hash_table->NA, node, NEXT_INHASH);
            Node_set(hash_table->NA, last_node, NEXT_INHASH, NULL);
        }
        hash_table->bins[i] = NULL;
    }
    //printf("\nbins cleared");
    
    Node ** new_bins = (Node **) CL_REALLOC(hash_table->bins, sizeof(Node *) * capacity);
    
    if (!new_bins) {
        result = CL_FAILURE;
    } else {
        //printf("\nmemory allocated");

        hash_table->bins = new_bins;
        // clear the new bins
        for (size_t i = hash_table->capacity; i < capacity; i++) {
            hash_table->bins[i] = NULL;
        }

        // set new capacity
        hash_table->capacity = capacity;
    }

    // re-hash the keys back into the bins, proceeding in order
    node = hash_table->head_inorder;
    while (node) {
        hash_t bin = hash_table->hash(Node_get(hash_table->NA, node, KEY), hash_table->capacity);
        last_node = hash_table->bins[bin];
        Node_set(hash_table->NA, node, NEXT_INHASH, last_node);
        hash_table->bins[bin] = node;
        node = Node_get(hash_table->NA, node, NEXT_INORDER);
    }
    //printf("\nnodes reset...returning");
    return result;
}

LinkedHashTable * LinkedHashTable_new(hash_t (*hash) (const void *, size_t), int (*comp) (const void *, const void *), size_t capacity, float max_load_factor, unsigned int flags, int narg_pairs, ...) {
    LinkedHashTable * hash_table = (LinkedHashTable *) CL_MALLOC(sizeof(LinkedHashTable));
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

    if (!NA->defaults) { // if default fails, not necessarily a problem
        NA->defaults = DEFAULT_NODE;
    }

    LinkedHashTable_init(hash_table, hash, comp, capacity, max_load_factor, NA);
    
    
    return hash_table;
}

void LinkedHashTable_init(LinkedHashTable * hash_table, hash_t (*hash) (const void *, size_t), int (*comp) (const void *, const void *), size_t capacity, float max_load_factor, NodeAttributes * NA) {
    hash_table->NA = NA;
    hash_table->head_inorder = NULL;
    hash_table->tail_inorder = NULL;
    hash_table->capacity = capacity;
    hash_table->size = 0;
    hash_table->max_load_factor = max_load_factor;
    hash_table->comp = comp;
    hash_table->hash = hash;

    for (size_t i = 0; i < capacity; i++) {
        hash_table->bins[i] = NULL;
    }
}
void LinkedHashTable_del(LinkedHashTable * hash_table) {
    while (hash_table->tail_inorder) {
        Node * node = hash_table->tail_inorder;
        hash_table->tail_inorder = Node_get(hash_table->NA, node, PREV_INORDER);
        if (hash_table->tail_inorder) {
            Node_set(hash_table->NA, hash_table->tail_inorder, NEXT_INORDER, NULL);
        }
        Node_del(node);
        hash_table->size--;
    }
    NodeAttributes_del(hash_table->NA);
    hash_table->NA = NULL;
    CL_FREE(hash_table->bins);
    hash_table->bins = NULL;
    CL_FREE(hash_table);
}

Node * LinkedHashTable_get_node(LinkedHashTable * hash_table, void * key) {
    Node * node = hash_table->bins[hash_table->hash(key, hash_table->capacity)];
    while (node && hash_table->comp(Node_get(hash_table->NA, node, KEY), key)) {
        node = Node_get(hash_table->NA, node, NEXT_INHASH);
    }
    return node;
}

int LinkedHashTable_set(LinkedHashTable * hash_table, void * key, void * value) {
    Node * node = LinkedHashTable_get_node(hash_table, key);
    // if node is found, overwrite the value
    if (node) {
        Node_set(hash_table->NA, node, VALUE, value);
        return CL_SUCCESS;
    }

    // create new node and assign it to bins and linked list
    hash_t bin = hash_table->hash(key, hash_table->capacity);
    node = Node_new(hash_table->NA, 0);
    
    if (!node) {
        return CL_FAILURE;
    }
    Node_set(hash_table->NA, node, KEY, key);
    Node_set(hash_table->NA, node, VALUE, value);
    if (!hash_table->size) {
        hash_table->head_inorder = node;
        hash_table->tail_inorder = node;
    } else {
        Node_set(hash_table->NA, node, NEXT_INHASH, hash_table->bins[bin]);
        Node_set(hash_table->NA, node, PREV_INORDER, hash_table->tail_inorder);
        Node_set(hash_table->NA, hash_table->tail_inorder, NEXT_INORDER, node);
        hash_table->tail_inorder = node;
    }
    hash_table->bins[bin] = node;

    hash_table->size++;

    if (((float)hash_table->size) / hash_table->capacity > hash_table->max_load_factor) {
        LinkedHashTable_resize(hash_table, next_prime(hash_table->capacity*2));
    }

    return CL_SUCCESS;
}

void * LinkedHashTable_get(LinkedHashTable * hash_table, void * key) {
    Node * node = LinkedHashTable_get_node(hash_table, key);
    if (node) {
        return Node_get(hash_table->NA, node, VALUE);
    }
    return NULL;
}

bool LinkedHashTable_contains(LinkedHashTable * hash_table, void * key) {
    return LinkedHashTable_get_node(hash_table, key) != NULL;
}

// remove and return the node identified by the key in hash_table. Returns NULL if key is not found
static Node * LinkedHashTable_pop_(LinkedHashTable * hash_table, void * key) {
    //printf("\nin LinkedHashTable_pop_");
    Node * last_node = NULL, * node, * next_node = NULL;
    hash_t bin = hash_table->hash(key, hash_table->capacity);
    node = hash_table->bins[bin];
    while (node && hash_table->comp(Node_get(hash_table->NA, node, KEY), key)) {
        last_node = node;
        node = Node_get(hash_table->NA, node, NEXT_INHASH);
    }

    if (!node) {
        return NULL; // failure to remove that which is not present
    }

    //printf("\nafter finding node\n\tlast_node: %p\n\tnode: %p", (void*)last_node, (void*)node);

    // node is now the node to be removed
    // last_node is the node preceding the node to be removed in the bin
    if (last_node) {
        //printf("\n\tlast_node->next_inhash: %p", (void*)last_node->next_inhash);
        Node_set(hash_table->NA, last_node, NEXT_INHASH, Node_get(hash_table->NA, node, NEXT_INHASH));
    } else { // node to be removed is stored in the bin
        hash_table->bins[bin] = Node_get(hash_table->NA, node, NEXT_INHASH);
    }
    //printf("\n\tnode->next_inhash: %p", (void*)node->next_inhash);

    // node is now removed from hash_table->bins, need to remove from linked list
    last_node = Node_get(hash_table->NA, node, PREV_INORDER);
    Node_set(hash_table->NA, node, PREV_INORDER, NULL);
    next_node = Node_get(hash_table->NA, node, NEXT_INORDER);
    Node_set(hash_table->NA, node, NEXT_INORDER, NULL);
    //printf("\nafter removing node from linked list\n\tlast_node: %p\n\tnext_node: %p", (void*)last_node, (void*)next_node);
    //printf("\n\tnode->prev_inorder: %p\n\tnode->next_inorder: %p", (void*)node->prev_inorder, (void*)node->next_inorder);
    if (last_node) {
        Node_set(hash_table->NA, last_node, NEXT_INORDER, next_node);
    }
    if (next_node) {
        Node_set(hash_table->NA, next_node, PREV_INORDER, last_node);
    }

    //printf("\n\thash_table->head_inorder: %p\n\thash_table->tail_inorder: %p", (void*)hash_table->head_inorder, (void*)hash_table->tail_inorder);

    // reset ends of linked list if necessary

    if (hash_table->head_inorder == node) {
        hash_table->head_inorder = next_node;
    }
    if (hash_table->tail_inorder == node) {
        hash_table->tail_inorder = last_node;
    }

    hash_table->size--;

    // TODO: check load factor and resize if necessary

    return node;
}

// removes and destroys the node identified by key. returns the value at the node. Warning, NULL is a perfectly acceptable value to store
// so to verify it is removed, you have to either check contains or use LinkedHashTable_remove and check for failure
void * LinkedHashTable_pop(LinkedHashTable * hash_table, void * key) {
    Node * to_rem = LinkedHashTable_pop_(hash_table, key);
    if (!to_rem) {
        return NULL;
    }
    void * val = Node_get(hash_table->NA, to_rem, VALUE);
    Node_del(to_rem);
    return val;
}

// removes and destroys the node identified by key. returns 0 if successful (key is found)
int LinkedHashTable_remove(LinkedHashTable * hash_table, void * key) {
    Node * to_rem = LinkedHashTable_pop_(hash_table, key);
    if (!to_rem) {
        return CL_FAILURE;
    }
    Node_del(to_rem);
    return CL_SUCCESS;
}

// ITERATORS:

LinkedHashTableKeyIterator * LinkedHashTable_keys(LinkedHashTable * hash_table, bool reversed) {
    return LinkedHashTableKeyIterator_new(hash_table, reversed);
}
LinkedHashTableValueIterator * LinkedHashTable_values(LinkedHashTable * hash_table, bool reversed) {
    return LinkedHashTableValueIterator_new(hash_table, reversed);
}
LinkedHashTableItemIterator * LinkedHashTable_items(LinkedHashTable * hash_table, bool reversed) {
    return LinkedHashTableItemIterator_new(hash_table, reversed);
}

LinkedHashTableKeyIterator * LinkedHashTableKeyIterator_new(LinkedHashTable * hash_table, bool reversed) {
    LinkedHashTableKeyIterator * key_iter = (LinkedHashTableKeyIterator *) CL_MALLOC(sizeof(LinkedHashTableKeyIterator));
    if (!key_iter) {
        return NULL;
    }
    LinkedHashTableKeyIterator_init(key_iter, hash_table, reversed);
    return key_iter;
}
LinkedHashTableValueIterator * LinkedHashTableValueIterator_new(LinkedHashTable * hash_table, bool reversed) {
    LinkedHashTableValueIterator * value_iter = (LinkedHashTableValueIterator *) CL_MALLOC(sizeof(LinkedHashTableValueIterator));
    if (!value_iter) {
        return NULL;
    }
    LinkedHashTableValueIterator_init(value_iter, hash_table, reversed);
    return value_iter;
}
LinkedHashTableItemIterator * LinkedHashTableItemIterator_new(LinkedHashTable * hash_table, bool reversed) {
    LinkedHashTableItemIterator * item_iter = (LinkedHashTableItemIterator *) CL_MALLOC(sizeof(LinkedHashTableItemIterator));
    if (!item_iter) {
        return NULL;
    }

    item_iter->next_item = DictItem_new(NULL, NULL);
    if (!item_iter->next_item) {
        CL_FREE(item_iter);
        return NULL;
    }
    LinkedHashTableItemIterator_init(item_iter, hash_table, reversed);
    return item_iter;
}
void LinkedHashTableKeyIterator_init(LinkedHashTableKeyIterator * key_iter, LinkedHashTable * hash_table, bool reversed) {
    key_iter->next_key = NULL;
    if (reversed) {
        key_iter->node = hash_table->tail_inorder;
    } else {
        key_iter->node = hash_table->head_inorder;
    }
    key_iter->NA = hash_table->NA;
    key_iter->reversed = reversed;
    key_iter->stop = ITERATOR_GO;
}
void LinkedHashTableValueIterator_init(LinkedHashTableValueIterator * value_iter, LinkedHashTable * hash_table, bool reversed) {
    value_iter->next_value = NULL;
    if (reversed) {
        value_iter->node = hash_table->tail_inorder;
    } else {
        value_iter->node = hash_table->head_inorder;
    }
    value_iter->NA = hash_table->NA;
    value_iter->reversed = reversed;
    value_iter->stop = ITERATOR_GO;
}
void LinkedHashTableItemIterator_init(LinkedHashTableItemIterator * item_iter, LinkedHashTable * hash_table, bool reversed) {
    item_iter->next_item->key = NULL;
    item_iter->next_item->value = NULL;
    if (reversed) {
        item_iter->node = hash_table->tail_inorder;
    } else {
        item_iter->node = hash_table->head_inorder;
    }
    item_iter->NA = hash_table->NA;
    item_iter->reversed = reversed;
    item_iter->stop = ITERATOR_GO;
}
void LinkedHashTableKeyIterator_del(LinkedHashTableKeyIterator * key_iter) {
    key_iter->next_key = NULL;
    key_iter->node = NULL;
    key_iter->NA = NULL;
    CL_FREE(key_iter);
}
void LinkedHashTableValueIterator_del(LinkedHashTableValueIterator * value_iter) {
    value_iter->next_value = NULL;
    value_iter->node = NULL;
    value_iter->NA = NULL;
    CL_FREE(value_iter);
}
void LinkedHashTableItemIterator_del(LinkedHashTableItemIterator * item_iter) {
    DictItem_del(item_iter->next_item);
    item_iter->next_item = NULL;
    item_iter->node = NULL;
    item_iter->NA = NULL;
    CL_FREE(item_iter);
}

LinkedHashTableKeyIterator * LinkedHashTableKeyIterator_iter(LinkedHashTable * hash_table, bool reversed) {
    return LinkedHashTableKeyIterator_new(hash_table, reversed);
}
LinkedHashTableValueIterator * LinkedHashTableValueIterator_iter(LinkedHashTable * hash_table, bool reversed) {
    return LinkedHashTableValueIterator_new(hash_table, reversed);
}
LinkedHashTableItemIterator * LinkedHashTableItemIterator_iter(LinkedHashTable * hash_table, bool reversed) {
    return LinkedHashTableItemIterator_new(hash_table, reversed);
}
const void * LinkedHashTableKeyIterator_next(LinkedHashTableKeyIterator * key_iter) {
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
void * LinkedHashTableValueIterator_next(LinkedHashTableValueIterator * value_iter) {
    if (!value_iter) {
        return NULL;
    }
    if (!value_iter->node) {
        value_iter->stop = ITERATOR_STOP;
        return NULL;
    }
    value_iter->next_value = Node_get(value_iter->NA, value_iter->node, VALUE);
    if (value_iter->reversed) {
        value_iter->node = Node_get(value_iter->NA, value_iter->node, PREV_INORDER);
    } else {
        value_iter->node = Node_get(value_iter->NA, value_iter->node, NEXT_INORDER);
    }
    return value_iter->next_value;
}
DictItem * LinkedHashTableItemIterator_next(LinkedHashTableItemIterator * item_iter)  {
    if (!item_iter) {
        return NULL;
    }
    if (!item_iter->node) {
        item_iter->stop = ITERATOR_STOP;
        return NULL;
    }
    item_iter->next_item->key = Node_get(item_iter->NA, item_iter->node, KEY);
    item_iter->next_item->value = Node_get(item_iter->NA, item_iter->node, VALUE);
    if (item_iter->reversed) {
        item_iter->node = Node_get(item_iter->NA, item_iter->node, PREV_INORDER);
    } else {
        item_iter->node = Node_get(item_iter->NA, item_iter->node, NEXT_INORDER);
    }
    return item_iter->next_item;
}
enum iterator_status LinkedHashTableKeyIterator_stop(LinkedHashTableKeyIterator * key_iter) {
    if (!key_iter) {
        return ITERATOR_STOP;
    }
    if (key_iter->stop == ITERATOR_STOP) {
        LinkedHashTableKeyIterator_del(key_iter);
        return ITERATOR_STOP;
    }
    return key_iter->stop;
}
enum iterator_status LinkedHashTableValueIterator_stop(LinkedHashTableValueIterator * value_iter) {
    if (!value_iter) {
        return ITERATOR_STOP;
    }
    if (value_iter->stop == ITERATOR_STOP) {
        LinkedHashTableValueIterator_del(value_iter);
        return ITERATOR_STOP;
    }
    return value_iter->stop;
}
enum iterator_status LinkedHashTableItemIterator_stop(LinkedHashTableItemIterator * item_iter) {
    if (!item_iter) {
        return ITERATOR_STOP;
    }
    if (item_iter->stop == ITERATOR_STOP) {
        LinkedHashTableItemIterator_del(item_iter);
        return ITERATOR_STOP;
    }
    return item_iter->stop;
}
