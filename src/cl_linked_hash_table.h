#include <stdint.h>
#include <stdbool.h>
#include "cl_core.h"
#include "cl_node.h"

#ifndef LINKED_HASH_TABLE_H
#define LINKED_HASH_TABLE_H

// TODO: eventually use macros to set the hash sizes so that user can replace hash function with a different output size
// TODO: incorporate a randomized hash seed for strings, "salting": see http://ocert.org/advisories/ocert-2011-003.html, referenced from https://docs.python.org/3.8/reference/datamodel.html#object.__hash__

#ifndef LINKED_HASH_TABLE_LOAD_FACTOR 
#define LINKED_HASH_TABLE_LOAD_FACTOR .75
#endif

#ifndef LINKED_HASH_TABLE_DEFAULT_CAPACITY
#define LINKED_HASH_TABLE_DEFAULT_CAPACITY 13
#endif

#ifndef LINKED_HASH_TABLE_SCALE_FACTOR
#define LINKED_HASH_TABLE_SCALE_FACTOR 2
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

typedef struct DoubleLinkedHashNode DoubleLinkedHashNode;
typedef struct LinkedHashTable LinkedHashTable;
typedef struct DictItem DictItem;
typedef struct LinkedHashTableKeyIterator LinkedHashTableKeyIterator;
typedef struct LinkedHashTableValueIterator LinkedHashTableValueIterator;
typedef struct LinkedHashTableItemIterator LinkedHashTableItemIterator;

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

//used only for item iterator
struct DictItem {
    const void * key;
    void * value;
};

/*
// this can probably just be replaced by a generic node from my containerlibs
struct DoubleLinkedHashNode {
    const void * key;
    void * value;
    DoubleLinkedHashNode * next_inorder; // next in linked list of HashTable Keys as inserted
    DoubleLinkedHashNode * next_inhash; // next in linked list of HashTable Keys as inserted that have the same hash
    DoubleLinkedHashNode * prev_inorder; // previous in linked list of HashTable Keys as inserted
};
*/

DictItem * DictItem_new(void * key, void * value);
void DictItem_init(DictItem * di, void * key, void * value);
void DictItem_del(DictItem * di);

/*
DoubleLinkedHashNode * DoubleLinkedHashNode_new(void * key, void * value, DoubleLinkedHashNode * next_inhash, DoubleLinkedHashNode * next_inorder, DoubleLinkedHashNode * prev_inorder);
void DoubleLinkedHashNode_init(DoubleLinkedHashNode * node, void * key, void * value, DoubleLinkedHashNode * next_inhash, DoubleLinkedHashNode * next_inorder, DoubleLinkedHashNode * prev_inorder);
void DoubleLinkedHashNode_del(DoubleLinkedHashNode * node);
*/

LinkedHashTable * LinkedHashTable_new(hash_t (*hash) (const void *, size_t), int (*comp) (const void *, const void *), size_t capacity, float max_load_factor, unsigned int flags, int narg_pairs, ...);
void LinkedHashTable_init(LinkedHashTable * hash_table, hash_t (*hash) (const void *, size_t), int (*comp) (const void *, const void *), size_t capacity, float max_load_factor, NodeAttributes * NA);
void LinkedHashTable_del(LinkedHashTable * hash_table);
int LinkedHashTable_set(LinkedHashTable * hash_table, void * key, void * value);
//DoubleLinkedHashNode * LinkedHashTable_get_node(LinkedHashTable * hash_table, void * key);
void * LinkedHashTable_get(LinkedHashTable * hash_table, void * key);
bool LinkedHashTable_contains(LinkedHashTable * hash_table, void * key);
size_t LinkedHashTable_size(LinkedHashTable * hash_table);
size_t LinkedHashTable_capacity(LinkedHashTable * hash_table);

// removes and destroys the node identified by key
// copy this to pop and replace remove algorithm with pop and ignoring the output
int LinkedHashTable_remove(LinkedHashTable * hash_table, void * key);
void * LinkedHashTable_pop(LinkedHashTable * hash_table, void * key);
int LinkedHashTable_resize(LinkedHashTable * hash_table, size_t capacity);
LinkedHashTableKeyIterator * LinkedHashTable_keys(LinkedHashTable * hash_table, bool reversed);
LinkedHashTableValueIterator * LinkedHashTable_values(LinkedHashTable * hash_table, bool reversed);
LinkedHashTableItemIterator * LinkedHashTable_items(LinkedHashTable * hash_table, bool reversed);

LinkedHashTableKeyIterator * LinkedHashTableKeyIterator_new(LinkedHashTable * hash_table, bool reversed);
LinkedHashTableValueIterator * LinkedHashTableValueIterator_new(LinkedHashTable * hash_table, bool reversed);
LinkedHashTableItemIterator * LinkedHashTableItemIterator_new(LinkedHashTable * hash_table, bool reversed);
void LinkedHashTableKeyIterator_init(LinkedHashTableKeyIterator * key_iter, LinkedHashTable * hash_table, bool reversed);
void LinkedHashTableValueIterator_init(LinkedHashTableValueIterator * value_iter, LinkedHashTable * hash_table, bool reversed);
void LinkedHashTableItemIterator_init(LinkedHashTableItemIterator * item_iter, LinkedHashTable * hash_table, bool reversed);
void LinkedHashTableKeyIterator_del(LinkedHashTableKeyIterator * key_iter);
void LinkedHashTableValueIterator_del(LinkedHashTableValueIterator * value_iter);
void LinkedHashTableItemIterator_del(LinkedHashTableItemIterator * item_iter);

LinkedHashTableKeyIterator * LinkedHashTableKeyIterator_iter(LinkedHashTable * hash_table, bool reversed);
LinkedHashTableValueIterator * LinkedHashTableValueIterator_iter(LinkedHashTable * hash_table, bool reversed);
LinkedHashTableItemIterator * LinkedHashTableItemIterator_iter(LinkedHashTable * hash_table, bool reversed);
const void * LinkedHashTableKeyIterator_next(LinkedHashTableKeyIterator * key_iter);
void * LinkedHashTableValueIterator_next(LinkedHashTableValueIterator * value_iter);
DictItem * LinkedHashTableItemIterator_next(LinkedHashTableItemIterator * item_iter);
enum iterator_status LinkedHashTableKeyIterator_stop(LinkedHashTableKeyIterator * key_iter);
enum iterator_status LinkedHashTableValueIterator_stop(LinkedHashTableValueIterator * value_iter);
enum iterator_status LinkedHashTableItemIterator_stop(LinkedHashTableItemIterator * item_iter);

#endif // LINKED_HASH_TABLE_H
