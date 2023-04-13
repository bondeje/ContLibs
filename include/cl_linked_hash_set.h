#include <stdint.h>
#include <stdbool.h>
#include "cl_core.h"
#include "cl_node.h"
#include "cl_hash_utils.h"

#ifndef LINKED_HASH_SET_H
#define LINKED_HASH_SET_H

// TODO: eventually use macros to set the hash sizes so that user can replace hash function with a different output size
// TODO: incorporate a randomized hash seed for strings, "salting": see http://ocert.org/advisories/ocert-2011-003.html, referenced from https://docs.python.org/3.8/reference/datamodel.html#object.__hash__

// TODO: incorporate optional behavior into a flag parameter

#ifndef LINKED_HASH_SET_LOAD_FACTOR 
#define LINKED_HASH_SET_LOAD_FACTOR .75
#endif

#ifndef LINKED_HASH_SET_DEFAULT_CAPACITY
#define LINKED_HASH_SET_DEFAULT_CAPACITY 13
#endif

#ifndef LINKED_HASH_SET_SCALE_FACTOR
#define LINKED_HASH_SET_SCALE_FACTOR 2
#endif

// set hash to NULL makes keys interpreted
typedef struct LinkedHashSet {
    NodeAttributes * NA;
    Node * head;
    Node * tail;
    Node ** bins;
    size_t size; // number of elements in hash_set
    size_t capacity; // allocation of hash_set, should be prime
    float max_load_factor;
    int (*comp) (const void *, const void *);
    hash_t (*hash) (const void *, size_t);
} LinkedHashSet;

typedef struct LinkedHashSetIterator {
    NodeAttributes * NA;
    Node * node;
    const void * next_key;
    enum iterator_status stop;
} LinkedHashSetIterator;

LinkedHashSet * LinkedHashSet_new(hash_t (*hash) (const void *, size_t), int (*comp) (const void *, const void *), size_t capacity, float max_load_factor, unsigned int flags, int narg_pairs, ...);
void LinkedHashSet_init(LinkedHashSet * hash_set, hash_t (*hash) (const void *, size_t), int (*comp) (const void *, const void *), size_t capacity, float max_load_factor, NodeAttributes * NA);
void LinkedHashSet_del(LinkedHashSet * hash_set);
int LinkedHashSet_add(LinkedHashSet * hash_set, void * key);
bool LinkedHashSet_contains(LinkedHashSet * hash_set, void * key);
size_t LinkedHashSet_size(LinkedHashSet * hash_set);
size_t LinkedHashSet_capacity(LinkedHashSet * hash_set);
int LinkedHashSet_remove(LinkedHashSet * hash_set, void * key);
int LinkedHashSet_resize(LinkedHashSet * hash_set, size_t capacity);

LinkedHashSetIterator * LinkedHashSetIterator_new(LinkedHashSet * hash_set);
void LinkedHashSetIterator_init(LinkedHashSetIterator * iter, LinkedHashSet * hash_set);
void LinkedHashSetIterator_del(LinkedHashSetIterator * iter);

const void * LinkedHashSetIterator_next(LinkedHashSetIterator * iter);
enum iterator_status LinkedHashSetIterator_stop(LinkedHashSetIterator * iter);

#endif // LINKED_HASH_SET_H
