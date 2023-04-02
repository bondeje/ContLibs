#include <stdint.h>
#include <stdbool.h>
#include "cl_core.h"
#include "cl_node.h"
#include "cl_hash_utils.h"

#ifndef LINKED_HASH_TABLE_H
#define LINKED_HASH_TABLE_H

// TODO: eventually use macros to set the hash sizes so that user can replace hash function with a different output size
// TODO: incorporate a randomized hash seed for strings, "salting": see http://ocert.org/advisories/ocert-2011-003.html, referenced from https://docs.python.org/3.8/reference/datamodel.html#object.__hash__

// TODO: incorporate optional behavior into a flag parameter

#ifndef LINKED_HASH_TABLE_LOAD_FACTOR 
#define LINKED_HASH_TABLE_LOAD_FACTOR .75
#endif

#ifndef LINKED_HASH_TABLE_DEFAULT_CAPACITY
#define LINKED_HASH_TABLE_DEFAULT_CAPACITY 13
#endif

#ifndef LINKED_HASH_TABLE_SCALE_FACTOR
#define LINKED_HASH_TABLE_SCALE_FACTOR 2
#endif

//typedef struct DoubleLinkedHashNode DoubleLinkedHashNode;
typedef struct LinkedHashTable LinkedHashTable;
typedef struct DictItem DictItem;

typedef struct LinkedHashTableKeyIterator {
    NodeAttributes * NA;
    Node * node;
    const void * next_key;
    enum iterator_status stop;
} LinkedHashTableKeyIterator;

typedef struct LinkedHashTableValueIterator {
    NodeAttributes * NA;
    Node * node;
    void * next_value;
    enum iterator_status stop;
} LinkedHashTableValueIterator;

typedef struct LinkedHashTableItemIterator {
    NodeAttributes * NA;
    Node * node;
    DictItem * next_item;
    enum iterator_status stop;
} LinkedHashTableItemIterator;

//used only for item iterator
struct DictItem {
    const void * key;
    void * value;
};

DictItem * DictItem_new(void * key, void * value);
void DictItem_init(DictItem * di, void * key, void * value);
void DictItem_del(DictItem * di);

LinkedHashTable * LinkedHashTable_new(hash_t (*hash) (const void *, size_t), int (*comp) (const void *, const void *), size_t capacity, float max_load_factor, unsigned int flags, int narg_pairs, ...);
void LinkedHashTable_init(LinkedHashTable * hash_table, hash_t (*hash) (const void *, size_t), int (*comp) (const void *, const void *), size_t capacity, float max_load_factor, NodeAttributes * NA);
void LinkedHashTable_del(LinkedHashTable * hash_table);
int LinkedHashTable_set(LinkedHashTable * hash_table, void * key, void * value);
//DoubleLinkedHashNode * LinkedHashTable_get_node(LinkedHashTable * hash_table, void * key);
void * LinkedHashTable_get(LinkedHashTable * hash_table, void * key);
bool LinkedHashTable_contains(LinkedHashTable * hash_table, void * key);
size_t LinkedHashTable_size(LinkedHashTable * hash_table);
size_t LinkedHashTable_capacity(LinkedHashTable * hash_table);
int LinkedHashTable_remove(LinkedHashTable * hash_table, void * key);
void * LinkedHashTable_pop(LinkedHashTable * hash_table, void * key);
int LinkedHashTable_resize(LinkedHashTable * hash_table, size_t capacity);
LinkedHashTableKeyIterator * LinkedHashTable_keys(LinkedHashTable * hash_table);
LinkedHashTableValueIterator * LinkedHashTable_values(LinkedHashTable * hash_table);
LinkedHashTableItemIterator * LinkedHashTable_items(LinkedHashTable * hash_table);

LinkedHashTableKeyIterator * LinkedHashTableKeyIterator_new(LinkedHashTable * hash_table);
LinkedHashTableValueIterator * LinkedHashTableValueIterator_new(LinkedHashTable * hash_table);
LinkedHashTableItemIterator * LinkedHashTableItemIterator_new(LinkedHashTable * hash_table);
void LinkedHashTableKeyIterator_init(LinkedHashTableKeyIterator * key_iter, LinkedHashTable * hash_table);
void LinkedHashTableValueIterator_init(LinkedHashTableValueIterator * value_iter, LinkedHashTable * hash_table);
void LinkedHashTableItemIterator_init(LinkedHashTableItemIterator * item_iter, LinkedHashTable * hash_table);
void LinkedHashTableKeyIterator_del(LinkedHashTableKeyIterator * key_iter);
void LinkedHashTableValueIterator_del(LinkedHashTableValueIterator * value_iter);
void LinkedHashTableItemIterator_del(LinkedHashTableItemIterator * item_iter);

const void * LinkedHashTableKeyIterator_next(LinkedHashTableKeyIterator * key_iter);
void * LinkedHashTableValueIterator_next(LinkedHashTableValueIterator * value_iter);
DictItem * LinkedHashTableItemIterator_next(LinkedHashTableItemIterator * item_iter);
enum iterator_status LinkedHashTableKeyIterator_stop(LinkedHashTableKeyIterator * key_iter);
enum iterator_status LinkedHashTableValueIterator_stop(LinkedHashTableValueIterator * value_iter);
enum iterator_status LinkedHashTableItemIterator_stop(LinkedHashTableItemIterator * item_iter);

#endif // LINKED_HASH_TABLE_H
