// https://en.wikipedia.org/wiki/Unrolled_linked_list
// unrolled is a pretty stupid name for this type of data structure. at best it is partially unrolled

#include <stddef.h>
#include <stdbool.h>
#include "cl_iterators.h"
#include "cl_dbl_linked_list.h"

#ifndef CL_HYBRID_LINKED_LIST_H
#define CL_HYBRID_LINKED_LIST_H

typedef struct HybridDblLinkedList {
    DblLinkedList dll;
    size_t max_elements;
} HybridDblLinkedList;

typedef struct HybridDblLinkedListIterator {
    Slice arr_iter;
    HybridDblLinkedList * hdll;
    Node * node;
    enum iterator_status stop;
} HybridDblLinkedListIterator, HybridDblLinkedListIteratorIterator;

HybridDblLinkedList * HybridDblLinkedList_new(unsigned int flags, int narg_pairs, ...);
void HybridDblLinkedList_init(HybridDblLinkedList * hdll, NodeAttributes * NA);
void HybridDblLinkedList_del(HybridDblLinkedList * hdll);
void HybridDblLinkedList_reverse(HybridDblLinkedList * hdll);
size_t HybridDblLinkedList_size(HybridDblLinkedList * hdll);
bool HybridDblLinkedList_is_empty(HybridDblLinkedList * hdll);
bool HybridDblLinkedList_contains(HybridDblLinkedList * hdll, void * value, int (*comp)(void*, void*));
enum cl_status HybridDblLinkedList_extend(HybridDblLinkedList * dest, HybridDblLinkedList * src);
void * HybridDblLinkedList_get(HybridDblLinkedList * hdll, size_t index);
void * HybridDblLinkedList_peek_front(HybridDblLinkedList * hdll);
void * HybridDblLinkedList_peek_back(HybridDblLinkedList * hdll);
enum cl_status HybridDblLinkedList_insert(HybridDblLinkedList * hdll, size_t loc, void * val);
enum cl_status HybridDblLinkedList_push_front(HybridDblLinkedList * hdll, void * val);
enum cl_status HybridDblLinkedList_push_back(HybridDblLinkedList * hdll, void * val);
void * HybridDblLinkedList_remove(HybridDblLinkedList * hdll, size_t loc);
void * HybridDblLinkedList_pop_front(HybridDblLinkedList * hdll);
void * HybridDblLinkedList_pop_back(HybridDblLinkedList * hdll);

// generally should only use this if the elements are unique in the value or you are sure you only want the first occurrence
// for all other use cases, the filter functionality is better.
void * HybridDblLinkedList_find(HybridDblLinkedList * hdll, void * value, int (*comp)(void*, void*));

//Iterators
void HybridDblLinkedListIterator_init(HybridDblLinkedListIterator * hdll_iter, HybridDblLinkedList * hdll);
void * HybridDblLinkedListIterator_next(HybridDblLinkedListIterator * hdll_iter);
enum iterator_status HybridDblLinkedListIterator_stop(HybridDblLinkedListIterator * hdll_iter);
void HybridDblLinkedListIteratorIterator_init(HybridDblLinkedListIteratorIterator * hdll_iter_iter, HybridDblLinkedListIterator * hdll_iter);
void * HybridDblLinkedListIteratorIterator_next(HybridDblLinkedListIteratorIterator * hdll_iter);
enum iterator_status HybridDblLinkedListIteratorIterator_stop(HybridDblLinkedListIteratorIterator * hdll_iter);

#endif // CL_HYBRID_LINKED_LIST_H