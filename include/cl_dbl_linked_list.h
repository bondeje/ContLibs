#include <stddef.h>
#include <stdbool.h>
#include "cl_linked_list.h"

#ifndef CL_DBL_LINKED_LIST_H
#define CL_DBL_LINKED_LIST_H

typedef struct DblLinkedList {
    LinkedList ll;
    Node * tail;
    bool reversed;
} DblLinkedList;

typedef struct DblLinkedListIterator {
    DblLinkedList * dll;
    Node * node;
    enum iterator_status stop;
} DblLinkedListIterator, DblLinkedListIteratorIterator;

DblLinkedList * DblLinkedList_new(unsigned int flags, int narg_pairs, ...);
void DblLinkedList_init(DblLinkedList * dll, NodeAttributes * NA);
extern void (*DblLinkedList_del)(DblLinkedList * dll);
void DblLinkedList_reverse(DblLinkedList * dll);

extern size_t (*DblLinkedList_size)(DblLinkedList *);
extern bool (*DblLinkedList_is_empty)(DblLinkedList *);
// generally should only use this if the elements are unique in the value or you are sure you only want the first occurrence
// for all other use cases, the filter functionality is better.
extern void * (*DblLinkedList_find)(DblLinkedList *, void *, int (*)(void*, void*));
extern bool (*DblLinkedList_contains)(DblLinkedList *, void *, int (*)(void*, void*));

enum cl_status DblLinkedList_extend(DblLinkedList * dest, DblLinkedList * src);
void * DblLinkedList_peek_front(DblLinkedList * dll);
void * DblLinkedList_peek_back(DblLinkedList * dll);
void * DblLinkedList_get(DblLinkedList * dll, size_t index);
enum cl_status DblLinkedList_insert(DblLinkedList * dll, size_t loc, void * val);
enum cl_status DblLinkedList_push_front(DblLinkedList * dll, void * val);
enum cl_status DblLinkedList_push_back(DblLinkedList * dll, void * val);
void * DblLinkedList_remove(DblLinkedList * dll, size_t loc);
void * DblLinkedList_pop_front(DblLinkedList * dll);
void * DblLinkedList_pop_back(DblLinkedList * dll);

//Iterators
void DblLinkedListIterator_init(DblLinkedListIterator * dll_iter, DblLinkedList * dll);
void * DblLinkedListIterator_next(DblLinkedListIterator * dll_iter);
enum iterator_status DblLinkedListIterator_stop(DblLinkedListIterator * dll_iter);
void DblLinkedListIteratorIterator_init(DblLinkedListIteratorIterator * dll_iter_iter, DblLinkedListIterator * dll_iter);
void * DblLinkedListIteratorIterator_next(DblLinkedListIteratorIterator * dll_iter);
enum iterator_status DblLinkedListIteratorIterator_stop(DblLinkedListIteratorIterator * dll_iter);

#endif // CL_DBLLINKEDLIST_H