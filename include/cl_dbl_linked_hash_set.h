#include <stdint.h>
#include <stdbool.h>
#include "cl_linked_hash_set.h"

#ifndef DBL_LINKED_HASH_SET_H
#define DBL_LINKED_HASH_SET_H

// TODO: eventually use macros to set the hash sizes so that user can replace hash function with a different output size
// TODO: incorporate a randomized hash seed for strings, "salting": see http://ocert.org/advisories/ocert-2011-003.html, referenced from https://docs.python.org/3.8/reference/datamodel.html#object.__hash__

// TODO: incorporate optional behavior into a flag parameter

#ifndef DBL_LINKED_HASH_SET_LOAD_FACTOR 
#define DBL_LINKED_HASH_SET_LOAD_FACTOR .75
#endif

#ifndef DBL_LINKED_HASH_SET_DEFAULT_CAPACITY
#define DBL_LINKED_HASH_SET_DEFAULT_CAPACITY 13
#endif

#ifndef DBL_LINKED_HASH_SET_SCALE_FACTOR
#define DBL_LINKED_HASH_SET_SCALE_FACTOR 2
#endif

typedef LinkedHashSet DblLinkedHashSet;

typedef LinkedHashSetIterator DblLinkedHashSetIterator;

DblLinkedHashSet * DblLinkedHashSet_new(hash_t (*hash) (const void *, size_t), int (*comp) (const void *, const void *), size_t capacity, float max_load_factor, unsigned int flags, int narg_pairs, ...);
void DblLinkedHashSet_init(DblLinkedHashSet * hash_set, hash_t (*hash) (const void *, size_t), int (*comp) (const void *, const void *), size_t capacity, float max_load_factor, NodeAttributes * NA);
void DblLinkedHashSet_del(DblLinkedHashSet * hash_set);
int DblLinkedHashSet_add(DblLinkedHashSet * hash_set, void * key);
bool DblLinkedHashSet_contains(DblLinkedHashSet * hash_set, void * key);
size_t DblLinkedHashSet_size(DblLinkedHashSet * hash_set);
size_t DblLinkedHashSet_capacity(DblLinkedHashSet * hash_set);
int DblLinkedHashSet_remove(DblLinkedHashSet * hash_set, void * key);
int DblLinkedHashSet_resize(DblLinkedHashSet * hash_set, size_t capacity);

DblLinkedHashSetIterator * DblLinkedHashSetIterator_new(DblLinkedHashSet * hash_set);
void DblLinkedHashSetIterator_init(DblLinkedHashSetIterator * iter, DblLinkedHashSet * hash_set);
void DblLinkedHashSetIterator_del(DblLinkedHashSetIterator * iter);

const void * DblLinkedHashSetIterator_next(DblLinkedHashSetIterator * iter);
enum iterator_status DblLinkedHashSetIterator_stop(DblLinkedHashSetIterator * iter);

#endif // DBL_LINKED_HASH_SET_H
