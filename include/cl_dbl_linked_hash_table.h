#include <stdint.h>
#include <stdbool.h>
#include "cl_linked_hash_table.h"

#ifndef DBL_LINKED_HASH_TABLE_H
#define DBL_LINKED_HASH_TABLE_H

// TODO: eventually use macros to set the hash sizes so that user can replace hash function with a different output size
// TODO: incorporate a randomized hash seed for strings, "salting": see http://ocert.org/advisories/ocert-2011-003.html, referenced from https://docs.python.org/3.8/reference/datamodel.html#object.__hash__

// TODO: incorporate optional behavior into a flag parameter

#ifndef DBL_LINKED_HASH_TABLE_LOAD_FACTOR 
#define DBL_LINKED_HASH_TABLE_LOAD_FACTOR .75
#endif

#ifndef DBL_LINKED_HASH_TABLE_DEFAULT_CAPACITY
#define DBL_LINKED_HASH_TABLE_DEFAULT_CAPACITY 13
#endif

#ifndef DBL_LINKED_HASH_TABLE_SCALE_FACTOR
#define DBL_LINKED_HASH_TABLE_SCALE_FACTOR 2
#endif

//typedef struct DoubleLinkedHashNode DoubleLinkedHashNode;
typedef LinkedHashTable DblLinkedHashTable;
typedef LinkedHashTableKeyIterator DblLinkedHashTableKeyIterator;
typedef LinkedHashTableItemIterator DblLinkedHashTableItemIterator;
typedef LinkedHashTableValueIterator DblLinkedHashTableValueIterator;

DblLinkedHashTable * DblLinkedHashTable_new(hash_t (*hash) (const void *, size_t), int (*comp) (const void *, const void *), size_t capacity, float max_load_factor, unsigned int flags, int narg_pairs, ...);
void DblLinkedHashTable_init(DblLinkedHashTable * hash_table, hash_t (*hash) (const void *, size_t), int (*comp) (const void *, const void *), size_t capacity, float max_load_factor, NodeAttributes * NA);
void DblLinkedHashTable_del(DblLinkedHashTable * hash_table);
int DblLinkedHashTable_set(DblLinkedHashTable * hash_table, void * key, void * value);
//DoubleLinkedHashNode * DblLinkedHashTable_get_node(DblLinkedHashTable * hash_table, void * key);
void * DblLinkedHashTable_get(DblLinkedHashTable * hash_table, void * key);
bool DblLinkedHashTable_contains(DblLinkedHashTable * hash_table, void * key);
size_t DblLinkedHashTable_size(DblLinkedHashTable * hash_table);
size_t DblLinkedHashTable_capacity(DblLinkedHashTable * hash_table);
int DblLinkedHashTable_remove(DblLinkedHashTable * hash_table, void * key);
void * DblLinkedHashTable_pop(DblLinkedHashTable * hash_table, void * key);
int DblLinkedHashTable_resize(DblLinkedHashTable * hash_table, size_t capacity);
DblLinkedHashTableKeyIterator * DblLinkedHashTable_keys(DblLinkedHashTable * hash_table);
DblLinkedHashTableValueIterator * DblLinkedHashTable_values(DblLinkedHashTable * hash_table);
DblLinkedHashTableItemIterator * DblLinkedHashTable_items(DblLinkedHashTable * hash_table);

DblLinkedHashTableKeyIterator * DblLinkedHashTableKeyIterator_new(DblLinkedHashTable * hash_table);
DblLinkedHashTableValueIterator * DblLinkedHashTableValueIterator_new(DblLinkedHashTable * hash_table);
DblLinkedHashTableItemIterator * DblLinkedHashTableItemIterator_new(DblLinkedHashTable * hash_table);
void DblLinkedHashTableKeyIterator_init(DblLinkedHashTableKeyIterator * key_iter, DblLinkedHashTable * hash_table);
void DblLinkedHashTableValueIterator_init(DblLinkedHashTableValueIterator * value_iter, DblLinkedHashTable * hash_table);
void DblLinkedHashTableItemIterator_init(DblLinkedHashTableItemIterator * item_iter, DblLinkedHashTable * hash_table);
void DblLinkedHashTableKeyIterator_del(DblLinkedHashTableKeyIterator * key_iter);
void DblLinkedHashTableValueIterator_del(DblLinkedHashTableValueIterator * value_iter);
void DblLinkedHashTableItemIterator_del(DblLinkedHashTableItemIterator * item_iter);

const void * DblLinkedHashTableKeyIterator_next(DblLinkedHashTableKeyIterator * key_iter);
void * DblLinkedHashTableValueIterator_next(DblLinkedHashTableValueIterator * value_iter);
DictItem * DblLinkedHashTableItemIterator_next(DblLinkedHashTableItemIterator * item_iter);
enum iterator_status DblLinkedHashTableKeyIterator_stop(DblLinkedHashTableKeyIterator * key_iter);
enum iterator_status DblLinkedHashTableValueIterator_stop(DblLinkedHashTableValueIterator * value_iter);
enum iterator_status DblLinkedHashTableItemIterator_stop(DblLinkedHashTableItemIterator * item_iter);

#endif // DBL_LINKED_HASH_TABLE_H
