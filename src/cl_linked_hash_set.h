#include <stdint.h>
#include <stdbool.h>
#include "cl_core.h"
#include "cl_node.h"

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

#ifndef HASH_OUT_BIT_SIZE
#define HASH_OUT_BIT_SIZE 64
#endif // HASH_OUT_BIT_SIZE

/* INTERNAL SETTINGS */

// leave undefined if HASH_OUT_BIT_SIZE > 64
#define hash_t

#if HASH_OUT_BIT_SIZE <= 64
    #undef hash_t
    #define hash_t unsigned long long
#endif

#if HASH_OUT_BIT_SIZE <= 32
    #undef hash_t
    #define hash_t unsigned long
#endif

#if HASH_OUT_BIT_SIZE <= 16
    #undef hash_t
    #define hash_t unsigned short
#endif

#if HASH_OUT_BIT_SIZE <= 8
    #undef hash_t
    #define hash_t unsigned char
#endif

typedef struct LinkedHashSet LinkedHashSet;
typedef struct DictItem DictItem;
typedef struct LinkedHashSetIterator LinkedHashSetIterator;

// simple djb
// type must be a valid identifier (cannot use *)
// need to verify that my type punning actually results in the correct bytes being read
#define BUILD_HASH_FUNCTION(type)                               \
static hash_t type##_hash(const void * key, size_t bin_size) {  \
    unsigned char bytes[sizeof(type)+1] = {'\0'};               \
    memcpy(bytes, key, sizeof(type));                           \
    return cstr_hash(bytes, bin_size);                          \
}

#define HASH_FUNCTION(type) type##_hash

hash_t cstr_hash(const void * key, size_t bin_size);
int cstr_comp(const void * a, const void * b);
hash_t address_hash(const void * val, size_t bin_size);
int address_comp(const void * a, const void * b);

LinkedHashSet * LinkedHashSet_new(hash_t (*hash) (const void *, size_t), int (*comp) (const void *, const void *), size_t capacity, float max_load_factor, unsigned int flags, int narg_pairs, ...);
void LinkedHashSet_init(LinkedHashSet * hash_set, hash_t (*hash) (const void *, size_t), int (*comp) (const void *, const void *), size_t capacity, float max_load_factor, NodeAttributes * NA);
void LinkedHashSet_del(LinkedHashSet * hash_set);
int LinkedHashSet_add(LinkedHashSet * hash_set, void * key);
bool LinkedHashSet_contains(LinkedHashSet * hash_set, void * key);
size_t LinkedHashSet_size(LinkedHashSet * hash_set);
size_t LinkedHashSet_capacity(LinkedHashSet * hash_set);
int LinkedHashSet_remove(LinkedHashSet * hash_set, void * key);
int LinkedHashSet_resize(LinkedHashSet * hash_set, size_t capacity);

LinkedHashSetIterator * LinkedHashSetIterator_new(LinkedHashSet * hash_set, bool reversed);
void LinkedHashSetIterator_init(LinkedHashSetIterator * iter, LinkedHashSet * hash_set, bool reversed);
void LinkedHashSetIterator_del(LinkedHashSetIterator * iter);

LinkedHashSetIterator * LinkedHashSetIterator_iter(LinkedHashSet * hash_set, bool reversed);
const void * LinkedHashSetIterator_next(LinkedHashSetIterator * iter);
enum iterator_status LinkedHashSetIterator_stop(LinkedHashSetIterator * iter);

#endif // LINKED_HASH_SET_H
